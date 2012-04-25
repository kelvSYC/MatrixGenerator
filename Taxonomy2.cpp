#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include "LookupTable.h"
#include "Taxonomy2.h"

/**
 * Functor evaluating Taxonomy 2 on a given input.  This is created as part of a cache lookup, and
 * is only invoked when there is a cache miss.
 *
 * In essence, this is the "core" of the Taxonomy 2 method.
 */
class Taxonomy2Evaluator {
	const boost::shared_ptr<const Discriminator> phi;
public:
	// We use this as a sparse vector stand-in
	typedef std::map<unsigned long, size_t> FrequencyVector;
	
	Taxonomy2Evaluator(const boost::shared_ptr<const Discriminator>& _phi) : phi(_phi) {}
	
	FrequencyVector operator()(const Subset& B) const {
		FrequencyVector result;	// indices range from 0 to fn->numOutputs() - 1, so it's likely this is sparse
		for (Subset::const_iterator it = B.begin(); it != B.end(); ++it) {
			Subset T(B.begin(), B.end());			// T = B ...
			T.erase(T.find(*it));					// ... - {*it}
			
			// Evaluate discriminaotr on subset and tally
			result[phi->evaluate(T)]++;
		}
		
		return result;
	}
};

/* **************************************************************************************************** */
/**
 * Table that caches the results of previous Taxonomy2 evaluation calls.
 */
class Taxonomy2LookupTable : public LookupTable<Taxonomy2Evaluator>::type {
	typedef LookupTable<Taxonomy2Evaluator>::type super_type;
	typedef EvaluationDelegate<Taxonomy2Evaluator> delegate_type;
public:
	Taxonomy2LookupTable(const Taxonomy2& fn) : super_type(delegate_type(fn.createEvaluator())) {}
};

/* **************************************************************************************************** */
class Taxonomy2EvalCache : public boost::noncopyable, public HeapValueBoostUnorderedMapCache<Taxonomy2, Taxonomy2LookupTable, HeapValueFromKeyInsertDelegate<Taxonomy2LookupTable> >::type {
	Taxonomy2EvalCache() {}
public:
	static Taxonomy2EvalCache& getInstance() {
		static Taxonomy2EvalCache instance;
		return instance;
	}
};

/* **************************************************************************************************** */
/**
 * Tests two Taxonomy2 for equality.
 *
 * Two Taxonomy2 over the same group are equal if and only if their two Discriminators are equal.
 */
bool Taxonomy2::equals(const GInvariant& rhs) const {
	if (!GInvariant::equals(rhs)) return false;
	const Taxonomy2& other = static_cast<const Taxonomy2&>(rhs);
	
	return *phi == *(other.phi);
}


std::size_t Taxonomy2::hash() const {
	std::size_t seed = 0;
	boost::hash_combine(seed, getGroup());
	boost::hash_combine(seed, *phi);		// Hash the discriminator, not the pointer
	return seed;
}

Taxonomy2::Evaluator Taxonomy2::createEvaluator() const {
	return Evaluator(phi);
}

/**
 * Evaluates the G-invariant function on an input subset.
 * 
 * Precondition: the size of the input subset MUST be one less than that of the discriminator size
 */
unsigned long Taxonomy2::evaluate(const Subset& B) const {
	Taxonomy2LookupTable& table = Taxonomy2EvalCache::getInstance().query(*this);
	
	return table.query(B);
}

bool Taxonomy2::hasCachedResult(const Subset& B) const {
	Taxonomy2LookupTable& table = Taxonomy2EvalCache::getInstance().query(*this);
	
	return table.contains(B);

}

std::deque<GInvariantEvaluationTask> Taxonomy2::getDependents(const Subset& B) const {
	std::deque<GInvariantEvaluationTask> results;
	
	// Taxonomy2 depends on the discriminator evaluation of every (B.size() - 1)-subset
	for (Subset::const_iterator it = B.begin(); it != B.end(); ++it) {
		Subset T(B.begin(), B.end());
		T.erase(T.find(*it));
		
		// Only insert uncached results
		if (!phi->hasCachedResult(T)) results.push_back(GInvariantEvaluationTask(boost::const_pointer_cast<Discriminator>(phi), T));
	}
	
	return results;
}