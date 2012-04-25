#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#ifndef CACHE_H
#define CACHE_H

// These are the default insert delegates, which insert a default-constructed Value into the table.  Unfortunately, due
// to limitations in C++03, this cannot be expanded into a version where you can store a boost::tuple as a data member
// and forward the tuple into the constructor of Value; this requires C++11.

template <class Value>
struct DefaultValueInsertDelegate {
	template<class Key>
	Value operator()(const Key&) const { return Value(); }
};

template <class Value>
struct HeapDefaultValueInsertDelegate {
	template<class Key>
	boost::shared_ptr<Value> operator()(const Key&) const { return boost::shared_ptr<Value>(new Value()); }
};

template <class Value>
struct ValueFromKeyInsertDelegate {
	template<class Key>
	Value operator()(const Key& key) const { return Value(key); }
};

template <class Value>
struct HeapValueFromKeyInsertDelegate {
	template<class Key>
	boost::shared_ptr<Value> operator()(const Key& key) const { return boost::shared_ptr<Value>(new Value(key)); }
};

/* ******************************************************************************************** */
// These are simple mappers that are used in CacheAdapters.  They are used to map the internal Key and Value types to
// the external Key and Value types.  Specifically, mappers used in mapKey() require that the functor take in a const Key&
// and return something convertible to MapType::key_type and mapValue() requires that the functor take in a
// const typename MapType::mapped_type& and return something convertible to a Value&

struct IdentityMapper {
	template <class T>
	const T& operator()(const T& t) const { return t; }
};

struct SharedPtrMapper {
	// TODO - enable_if subclass of boost::enable_shared_from_this<T>
	template <class T>
	boost::shared_ptr<T> operator()(const T& t) const { return boost::const_pointer_cast<T>(t.shared_from_this()); }
};

struct DereferenceMapper {
	template <class T>
	T& operator()(const boost::shared_ptr<T>& t) const { return *t; }
};

/* ******************************************************************************************** */
/**
 * Abstract superclass for all evaluation caches.
 *
 * @param <Key> The key type of the cache.  If using C++03, this may not be a reference type.
 * @param <Value> The value type of the cache.  If using C++03, this may not be a reference type.
 */
template <class Key, class Value>
struct Cache {
	typedef Key key_type;
	typedef Value mapped_type;
	
	virtual ~Cache() {}
	
	/**
	 * Queries the cache for the existence of the specified key.
	 */
	virtual bool contains(const key_type& key) = 0;
	
	/**
	 * Queries the cache.  If the key already exists in the cache, then the cached value is returned;
	 * else, a value is computed, placed in the cache, and returned.
	 */
	virtual mapped_type& query(const key_type& key) = 0;
};

/**
 * An abstract thread-safe evaluation cache, backed by an associative container.
 * The cache is thread-safe using read-write locking.
 *
 * @param <MapType> The type of the backing associative container.  The type should follow the same
 * 					conventions as std::map or boost::unordered_map.
 * @param <Delegate> The delegate functor used to create a new value from a key, which will be
 * 					 inserted into the table.
 */
template <class MapType, class Delegate = DefaultValueInsertDelegate<typename MapType::mapped_type> >
class MapCache : public Cache<typename MapType::key_type, typename MapType::mapped_type> {
	MapType cache;
	Delegate delegate;
	
	// Locking
	boost::shared_mutex mutex;
	typedef boost::shared_lock<boost::shared_mutex> ReadLock;
	typedef boost::upgrade_lock<boost::shared_mutex> RereadLock;
	typedef boost::upgrade_to_unique_lock<boost::shared_mutex> WriteLock;
public:
	// Don't you just hate it when you have to redeclare your public typedefs over again?
	// Even when Cache<typename MapType::key_type, typename MapType::mapped_type>::key_type = MapType::key_type;
	typedef typename MapType::key_type key_type;
	typedef typename MapType::mapped_type mapped_type;
	
	explicit MapCache(const Delegate& delegate_ = Delegate(), const MapType& cache_ = MapType()) : delegate(delegate_), cache(cache_) {}
	
	virtual ~MapCache() {}
	
	/**
	 * Queries the cache for the existence of the specified key.
	 */
	bool contains(const key_type& key) {
		ReadLock readLock(mutex);
		return cache.count(key) != 0;
	}
	
	/**
	 * Queries the cache.  If the key already exists in the cache, then the cached value is returned;
	 * else, a value is computed, placed in the cache, and returned.
	 */
	mapped_type& query(const key_type& key) {
		ReadLock readLock(mutex);					// Read Lock
		
		if (cache.count(key) == 0) {
			readLock.unlock();
			
			RereadLock rereadLock(mutex);			// Reread Lock
			if (cache.count(key) == 0) {			// If some other thread has not written into the cache while waiting
				WriteLock writeLock(rereadLock);	// Write Lock - wait again for all the readers to leave
				// Insert key to cache
				cache.insert(typename MapType::value_type(key, delegate(key)));
			}
			
			readLock = rereadLock;					// Downgrade to read lock
		}
		return cache[key];
	}
};

/**
 * Convenience class for an abstract MapCache backed by a std::map.
 *
 * This is to work around the fact that you can't do a "template typedef" in C++03.
 */
template <
	class Key, class Value,
	class Delegate = DefaultValueInsertDelegate<Value>,
	class Compare = std::less<Key>,
	class Allocator = std::allocator<std::pair<const Key, Value> >
	>
struct StdMapCache {
	typedef MapCache<std::map<Key, Value, Compare, Allocator>, Delegate> type;
};

/**
 * Convenience class for an abstract MapCache backed by a boost::unordered_map.
 * 
 * This is to work around the fact that you can't to a "template typedef" in C++03.
 */
template <
	class Key, class Value,
	class Delegate = DefaultValueInsertDelegate<Value>,
	class Hash = boost::hash<Key>, class Predicate = std::equal_to<Key>,
	class Allocator = std::allocator<std::pair<const Key, Value> >
	>
struct BoostUnorderedMapCache {
	typedef MapCache<boost::unordered_map<Key, Value, Hash, Predicate, Allocator>, Delegate> type;
};

/* **************************************************************************************************** */

/**
 * An abstract evaluation cache that is backed by an evaluation cache of different types.  This cache contains an internal
 * MapCache for its dirty work.
 *
 * @param <Key> The key type of the external cache
 * @param <Value> The value type of the external cache
 * @param <MapType> The MapType of the internal cache
 * @param <KeyMapper> A functor type that maps from the key type of the external cache to that of the internal cache.
 * @param <ValueMapper> A functor type that maps from the value type of the internal cache to that of the external cache.
 * @param <Delegate> The Delegate of the internal cache.
 */
template <class Key, class Value, class MapType, class KeyMapper, class ValueMapper, class Delegate = DefaultValueInsertDelegate<Value> >
class CacheAdapter2 : public Cache<Key, Value> {
	MapCache<MapType, Delegate> cache;
	KeyMapper keyMapper;
	ValueMapper valueMapper;
public:
	typedef Key key_type;
	typedef Value mapped_type;
	
	explicit CacheAdapter2(const Delegate& delegate = Delegate(), const KeyMapper& keyMapper_ = KeyMapper(), const ValueMapper& valueMapper_ = ValueMapper()) : cache(delegate), keyMapper(keyMapper_), valueMapper(valueMapper_) {}
	
	bool contains(const key_type& key) {
		const typename MapType::key_type& mappedKey = keyMapper(key);
		return cache.contains(mappedKey);
	}
	
	mapped_type& query(const key_type& key) {
		const typename MapType::key_type& mappedKey = keyMapper(key);
		typename MapType::mapped_type& mappedValue = cache.query(mappedKey);
		return valueMapper(mappedValue);
	}
};

/**
 * Convenience class for an evaluation cache whose values and keys must both be stored as pointers for whatever reason.
 * As it uses boost::shared_ptr for all of its smart pointer handling, it is backed by a std::map.
 *
 * This is to work around the fact that you can't do a "template typedef" in C++03.
 *
 * @param <Key> The outer key type.  Must extend boost::enable_shared_from_this<Key>.  Calls to shared_from_this() from an
 *              instance of Key must be well-defined.
 */
template <
	class Key, class Value,
	class Delegate = HeapDefaultValueInsertDelegate<Value>,
	class Compare = std::less<boost::shared_ptr<Key> >,
	class Allocator = std::allocator<std::pair<const boost::shared_ptr<Key>, boost::shared_ptr<Value> > >
>
struct HeapKeyHeapValueCache {
	typedef std::map<boost::shared_ptr<Key>, boost::shared_ptr<Value>, Compare, Allocator> inner_map_type;
	typedef CacheAdapter2<Key, Value, inner_map_type, SharedPtrMapper, DereferenceMapper, Delegate> type;
};

/**
 * Convenience class for an evaluation cache class backed by a std::map, whose value type must be heap allocated.  Thus,
 * the internal cache stores a boost::shared_ptr<Value> instead of the value type itself. No transformation is made to the
 * key type.
 *
 * This is to work around the fact that you can't do a "template typedef" in C++03.
 *
 * @param <Delegate> The Delegate of the internal cache.  The delegate functor should return boost::shared_ptr<Value>.
 */
template <
	class Key, class Value,
	class Delegate = HeapDefaultValueInsertDelegate<Value>,
	class Compare = std::less<Key>,
	class Allocator = std::allocator<std::pair<const Key, boost::shared_ptr<Value> > >
>
struct HeapValueStdMapCache {
	typedef std::map<Key, boost::shared_ptr<Value>, Compare, Allocator> inner_map_type;
	typedef CacheAdapter2<Key, Value, inner_map_type, IdentityMapper, DereferenceMapper, Delegate> type;
};

/**
 * Convenience class for an evaluation cache backed by a boost::unordered_map, whose value must be heap allocated.  Thus,
 * the internal cache stores a boost::shared_ptr<Value> instead of the value type itself. No transformation is made to the
 * key type.
 *
 * This is to work around the fact that you can't do a "template typedef" in C++03.
 *
 * @param <Delegate> The Delegate of the internal cache.  The delegate functor should return boost::shared_ptr<Value>.
 */
template <
	class Key, class Value,
	class Delegate = HeapDefaultValueInsertDelegate<Value>,
	class Hash = boost::hash<Key>, class Predicate = std::equal_to<Key>,
	class Allocator = std::allocator<std::pair<const Key, boost::shared_ptr<Value> > >
>
struct HeapValueBoostUnorderedMapCache {
	typedef boost::unordered_map<Key, boost::shared_ptr<Value>, Hash, Predicate, Allocator> inner_map_type;
	typedef CacheAdapter2<Key, Value, inner_map_type, IdentityMapper, DereferenceMapper, Delegate> type;
};
#endif
