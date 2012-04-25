#include "Cache.h"
#include "utils.h"

#ifndef LookupTable_h
#define LookupTable_h

/* This header file defines the concept of an Evaluator and its use in the GInvariant lookup tables.  Evaluators form
 * the core of the GInvariant, where results are actually computed.  Evaluators are not related in any way to each other,
 * but they do follow an informal protocol (in C++ parlance, a concept).  In particular, Evaluators are functors that also
 * abide by the following:
 *
 * * Evaluators must export a type, FrequencyVector.  This is the value returned from the functor's operator()().
 * * operator()() takes one argument of type const Subset&.  This is the input subset which is to be evaluated upon.
 * * Evaluators must be copyable.
 */

/**
 * Common cache delegate class for all G-Invariant subclasses, used in the evaluation caches.  It consists of a translation
 * table, which translates the FrequencyVectors returned by the Evaluator into the integers required by
 * GInvariant::evaluate() and its caches.
 */
template <class Evaluator>
class EvaluationDelegate {
	typedef typename Evaluator::FrequencyVector FrequencyVector;
public:
	explicit EvaluationDelegate(const Evaluator& eval_) : eval(eval_) {}
	
	unsigned long operator()(const Subset& key) {
		FrequencyVector fv = eval(key);
		
		// Look up in translation table - if not found, return a new value
		if (translator.count(fv) == 0) {
			translator[fv] = nextIdx;
			nextIdx++;
		}
		return translator[fv];
	}
private:
	Evaluator eval;
	
	// Translation table
	std::map<FrequencyVector, unsigned long> translator;
	unsigned long nextIdx;
};

/**
 * Convenience class for a lookup table, which is essentially a StdMapCache with an EvaluationDelegate.
 *
 * This is to work around the fact that you can't do a "template typedef" in C++03.
 */
template <class Evaluator>
struct LookupTable {
	typedef typename StdMapCache<Subset, unsigned long, EvaluationDelegate<Evaluator> >::type type;
};

/**
 * Delegate for inserting new LookupTable<Evaluator>::type as values in a cache.  Note that LookupTable<Evaluator>::type
 * is not copyable (as it is a MapCache, which isn't copyable), and EvaluationDelegate<Evaluator> is not
 * default-constructible.
 */
template <class Evaluator>
class EvaluatorInsertDelegate {
	Evaluator eval;
	
	typedef typename LookupTable<Evaluator>::type mapped_type;
public:
	EvaluatorInsertDelegate(const Evaluator& eval_) : eval(eval_) {}
	
	boost::shared_ptr<mapped_type> operator()(const Subset::size_type&) const {
		typedef EvaluationDelegate<Evaluator> delegate_type;
		
		return boost::shared_ptr<mapped_type>(new mapped_type(delegate_type(eval)));
	}
};

/**
 * Convenience class for a cache for GInvariants which work on multiple input sizes.
 */
template <class Evaluator>
class SizeIndependentLookupTable : public HeapValueStdMapCache<Subset::size_type, typename LookupTable<Evaluator>::type, EvaluatorInsertDelegate<Evaluator> >::type {
	typedef typename HeapValueStdMapCache<Subset::size_type, typename LookupTable<Evaluator>::type, EvaluatorInsertDelegate<Evaluator> >::type super_type;
public:
	typedef Subset::size_type key_type;
	typedef typename LookupTable<Evaluator>::type mapped_type;
	
	explicit SizeIndependentLookupTable(const Evaluator& eval) : super_type(EvaluatorInsertDelegate<Evaluator>(eval)) {}
};

#endif
