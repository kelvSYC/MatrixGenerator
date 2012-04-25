#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include "AnchorSet.h"
#include "LookupTable.h"

boost::mt19937 rng;

/* *********************************************************************************************** */
/**
 * Functor that evaluates an anchor set for a given input.  AnchorSet::evaluate() creates this
 * as part of cache lookup, and this is only invoked if the cached result is not found.
 *
 * This functor, in essence is the "core" of the Anchor Set method.
 */
class AnchorSetEvaluator {
	const boost::unordered_map<Permutation, Subset> imageSet;
public:
	typedef std::vector<unsigned long> FrequencyVector;
	
	explicit AnchorSetEvaluator(const boost::unordered_map<Permutation, Subset>& _imageSet)
		: imageSet(_imageSet) {}
	
	FrequencyVector operator()(const Subset& B) const {
		FrequencyVector v(B.size() + 1);
		
		typedef boost::function<Subset (boost::unordered_map<Permutation, Subset>::value_type)> Function;
		typedef boost::unordered_map<Permutation, Subset>::const_iterator BaseIterator;
		typedef boost::transform_iterator<Function, BaseIterator> Iterator;
		
		// Technically, the keys (elements in G) aren't needed, but just in case we need to know which element of G
		// we are currently iterating through, it remains an option.
		Function getSecond = boost::mem_fn(&boost::unordered_map<Permutation, Subset>::value_type::second);
		for (Iterator it = Iterator(imageSet.begin(), getSecond); it != Iterator(imageSet.end(), getSecond); ++it) {
			Subset intersection = setIntersection(*it, B);
			
			// Add to frequency vector v
			v[intersection.size()]++;
		}
		return v;
	}
};

/* *********************************************************************************************** */
/**
 * Table that caches the results of previous AnchorSet evaluation calls.
 */
class AnchorSetLookupTable : public SizeIndependentLookupTable<AnchorSetEvaluator> {
	typedef SizeIndependentLookupTable<AnchorSetEvaluator> super_type;
public:
	typedef super_type::mapped_type Subtable;
	
	explicit AnchorSetLookupTable(const AnchorSet& fn) : super_type(fn.createEvaluator()) {}
};

/* *********************************************************************************************** */
/**
 * Evaluation cache for AnchorSet::evaluate().  This is implemented as a "classic singleton", which
 * is thread-safe only in C++11.  It should be thread-safe under C++03 on gcc and clang, the
 * two compilers used in development.
 */
class AnchorSetEvalCache : public boost::noncopyable, public HeapValueStdMapCache<AnchorSet, AnchorSetLookupTable, HeapValueFromKeyInsertDelegate<AnchorSetLookupTable>, AnchorSetWeakOrdering>::type {
	// boost::noncopyable also implicitly deletes move constructor
	
public:
	static AnchorSetEvalCache& getInstance() {
		static AnchorSetEvalCache instance;
		return instance;
	}
private:
	AnchorSetEvalCache() {}
};

/* *********************************************************************************************** */
// AnchorSetWeakOrdering methods

bool AnchorSetWeakOrdering::operator()(const AnchorSet& lhs, const AnchorSet& rhs) const {
	if (GroupWeakOrdering()(lhs.getGroup(), rhs.getGroup())) return true;
	return lhs.getAnchorSet() < rhs.getAnchorSet();
}

/* *********************************************************************************************** */
// AnchorSet methods

/**
 * Creates an anchor set.  The anchor set itself is randomly generated.
 * For easy computation purposes there is a large initial overhead, caused by storing the image
 * of the anchor set from each element of G.
 *
 * Anchor sets will typically have low efficiency, but will improve as the size of the anchor set
 * approaches floor(v/2).  However, performance (especially in space) will suffer as k increases to
 * the same bound.  Anchor sets are applicable for any subset of X.
 *
 * @param G The permutation group the set is to be operate over.
 * @param v The largest element in the set X = {1, .., v}
 * @param size The size of the anchor set.
 */
AnchorSet::ptr AnchorSet::buildAnchorSet(const Group& G, size_t size) {
	Subset anchorSet;
	
	// Initialize random number generator
	// Permutation uses {0, .., v - 1} to represent X = {1, .., v}
	boost::uniform_int<long> distribution(0, G.getNumPoints() - 1);
	boost::variate_generator<boost::mt19937&, boost::uniform_int<long> > rand(rng, distribution);
	
	// Populate anchor set
	while (anchorSet.size() < size) {
		long candidate = rand();
		if (anchorSet.count(candidate) == 0) anchorSet.insert(candidate);
	}
	
	return AnchorSet::ptr(new AnchorSet(G, anchorSet));
}

AnchorSet::AnchorSet(const Group& _G, const Subset& _anchorSet) :
	GInvariant(_G), anchorSet(_anchorSet), imageSet() {
	// Build the image set
	for (GroupElementIterator it = G->elementsBegin(); it != G->elementsEnd(); it++) {
		Permutation g = *it;
		
		// apply g to anchorSet
		Subset image;
		std::transform(anchorSet.begin(), anchorSet.end(), std::inserter(image, image.begin()), boost::bind(&Permutation::at, g, _1));
		
		imageSet[g] = image;
	}
}

/**
 * Compares two AnchorSets for equality.
 * 
 * Two AnchorSets over the same group are equal if their sets are equal.
 */
bool AnchorSet::equals(const GInvariant& rhs) const {
	if (!GInvariant::equals(rhs)) return false;
	const AnchorSet& other = static_cast<const AnchorSet&>(rhs);
	return anchorSet == other.anchorSet;
}

std::size_t AnchorSet::hash() const {
	std::size_t seed = 0;
	boost::hash_combine(seed, getGroup());
	boost::hash_combine(seed, getAnchorSet());
	return seed;
}

AnchorSet::Evaluator AnchorSet::createEvaluator() const {
	return Evaluator(imageSet);
}

/**
 * Evaluates the G-invariant function on a given subset B of X = {1, .., v}.
 *
 * The rank (size of codomain) of this G-invariant function for a given size of B is at least
 * the largest returned value so far.
 *
 * @param B The input set.
 */
unsigned long AnchorSet::evaluate(const Subset& B) const {
	AnchorSetLookupTable& cache = AnchorSetEvalCache::getInstance().query(*this);
	AnchorSetLookupTable::Subtable& table = cache.query(B.size());
	
	return table.query(B);
}

bool AnchorSet::hasCachedResult(const Subset& B) const {
	AnchorSetLookupTable& cache = AnchorSetEvalCache::getInstance().query(*this);
	AnchorSetLookupTable::Subtable& table = cache.query(B.size());
	return table.contains(B);
}

