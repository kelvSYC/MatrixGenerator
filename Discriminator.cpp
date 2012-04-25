#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include "Discriminator.h"
#include "LookupTable.h"
#include "Taxonomy2.h"

/**
 * Functor that evaluates discriminators based on a given input.  This functor is created as part of
 * cache lookup, and invoked only when the cached result is not found.
 *
 * In essence, this is the "core" of the Discriminator.
 */
class DiscriminatorEvaluator {
public:
	typedef std::vector<unsigned long> FrequencyVector;
	
	DiscriminatorEvaluator(const std::vector<GInvariant::ptr>& _fns)
		: fns(_fns) {}
	
	FrequencyVector operator()(const Subset& B) const {
		FrequencyVector v(fns.size());
		
		std::transform(fns.begin(), fns.end(), v.begin(), boost::bind(&GInvariant::evaluate, _1, B));
		
		return v;
	}
private:
	const std::vector<GInvariant::ptr>& fns;
};

/* **************************************************************************************************** */
/**
 * Specialization of EvaluationDelegate, to account for the fact that the translation table is fully-built when the
 * Discriminator is created.
 */
template <>
class EvaluationDelegate<DiscriminatorEvaluator> {
	typedef DiscriminatorEvaluator::FrequencyVector FrequencyVector;
	typedef std::map<FrequencyVector, unsigned long> TranslatorTable;
public:
	EvaluationDelegate(const DiscriminatorEvaluator& eval_, TranslatorTable translator_) : eval(eval_), translator(translator_) {}
	
	unsigned long operator()(const Subset& key) {
		FrequencyVector fv = eval(key);
		
		return translator.find(fv)->second;		// The translator table is fully-built
	}
private:
	DiscriminatorEvaluator eval;
	std::map<FrequencyVector, unsigned long> translator;
};

/**
 * Simple structure storing both a lookup table as well as a Taxonomy2.  Each Discriminator is associated with one each
 * of both of these.  The Taxonomy2 is created whenever this is created, but is otherwise "lazy" (as in, only when
 * DiscriminatorEvalCache creates an entry for the Discriminator).
 */
class DiscriminatorEvalCacheEntry {
public:
	DiscriminatorEvalCacheEntry(const Discriminator& fn) : invariant(new Taxonomy2(fn.getGroup(), fn.shared_from_this())), resultCache(EvaluationDelegate<DiscriminatorEvaluator>(fn.createEvaluator(), fn.lookupTable), fn.newCache) {}
	
	boost::shared_ptr<Taxonomy2> getInvariant() const { return invariant; }
	LookupTable<DiscriminatorEvaluator>::type& getResultCache() { return resultCache; }
private:
	LookupTable<DiscriminatorEvaluator>::type resultCache;
	boost::shared_ptr<Taxonomy2> invariant;
};

/**
 * Alternative to GInvariantInsertDelegate to account for the unique Discriminator requirements.
 */
struct DiscriminatorInsertDelegate {
	boost::shared_ptr<DiscriminatorEvalCacheEntry> operator()(const boost::shared_ptr<Discriminator>& key) const {
		return boost::shared_ptr<DiscriminatorEvalCacheEntry>(new DiscriminatorEvalCacheEntry(*key));
	}
};

/**
 * Cache for Discriminator::evaluate() and Discriminator::invariant(), two Discriminator functions that are supposed to be
 * the same for all Discriminators for which operator==() returns true amongst each other.
 * 
 * Note that unlike other caches, this stores the Discriminator pointers as keys instead of the Discriminators themselves.
 * Keys in regular caches are required to be copyable, and this would prevent the proper retrieval of the Taxonomy2 when
 * needed (specifically, the adapter process creates a copy, and retrieving a shared_ptr from the copy will fail).
 * 
 * This is implemented as a "classic singleton", which is guaranteed thread-safe in C++11, but
 * is confined to single-threaded operation in earlier versions of C++.  It should be thread-safe under
 * C++03 on gcc and clang, the two compilers used in development.
 */
class DiscriminatorEvalCache : public boost::noncopyable, public HeapKeyHeapValueCache<Discriminator, DiscriminatorEvalCacheEntry, DiscriminatorInsertDelegate>::type {
	// boost::noncopyable also implicitly deletes move constructor
public:
	static DiscriminatorEvalCache& getInstance() {
		static DiscriminatorEvalCache instance;
		return instance;
	}
	
private:
	DiscriminatorEvalCache() {}
};

/* **************************************************************************************************** */
// Discriminator functions

/**
 * Constructs the Taxonomy 2 G-invariant function lazily.
 */
const GInvariant::ptr Discriminator::getInvariant() const {
	DiscriminatorEvalCacheEntry& cache = DiscriminatorEvalCache::getInstance().query(*const_cast<Discriminator*>(this));
	return cache.getInvariant();
}

/**
 * Tests two Discriminators for equality.
 *
 * Technically, two Discriminators over the same group are "semantically equal" if they work on the same
 * size of Subset, (ie. if both discriminate the same subset size), but under the GInvariant convention
 * but we need to ensure that if *this == rhs, then evaluate(B) == rhs.evaluate(B) for every appropriate
 * Subset B.  We don't know exactly when this occurs (there could be, and likely is, two different
 * function lists for which evaluate() returns the same values), so the best we can do is say that two 
 * Discriminators are equal if and only if they have the same function list.
 */
bool Discriminator::equals(const GInvariant& rhs) const {
	if (!GInvariant::equals(rhs)) return false;
	const Discriminator& other = static_cast<const Discriminator&>(rhs);
	
	// "Deep-compare" functions and other.functions for equality
	if (functions.size() != other.functions.size()) return false;
	for (int i = 0; i < functions.size(); i++) {
		if (!functions[i]->equals(*other.functions[i])) return false;
	}
	return true;
}

std::size_t Discriminator::hash() const {
	std::size_t seed = 0;
	boost::hash_combine(seed, getGroup());
	
	// Hash the actual functions, not the pointers to them
	typedef boost::indirect_iterator<GInvariantList::const_iterator> Iterator;
	boost::hash_range(seed, Iterator(functions.begin()), Iterator(functions.end()));
	
	return seed;
}

Discriminator::Evaluator Discriminator::createEvaluator() const {
	return Evaluator(functions);
}

unsigned long Discriminator::evaluate(const Subset& B) const {
	DiscriminatorEvalCacheEntry& cache = DiscriminatorEvalCache::getInstance().query(*const_cast<Discriminator*>(this));
	
	return cache.getResultCache().query(B);
}

bool Discriminator::hasCachedResult(const Subset& B) const {
	DiscriminatorEvalCacheEntry& cache = DiscriminatorEvalCache::getInstance().query(*const_cast<Discriminator*>(this));
	return cache.getResultCache().contains(B);
}

std::deque<GInvariantEvaluationTask> Discriminator::getDependents(const Subset& B) const {
	std::deque<GInvariantEvaluationTask> results;
	
	// Discriminator depends on the individual GInvariants that make up the Discriminator.
	for (std::vector<GInvariant::ptr>::const_iterator it = functions.begin(); it != functions.end(); ++it) {
		// Only insert uncached results
		if (!(*it)->hasCachedResult(B)) results.push_back(GInvariantEvaluationTask(*it, B));
	}
	
	return results;
}