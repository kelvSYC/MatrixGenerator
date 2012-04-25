#include "Discriminator.h"
#include "GInvariant.h"

#ifndef TAXONOMY2_H
#define TAXONOMY2_H

class Taxonomy2Evaluator;

// Taxonomy2 is centrally managed by DiscriminatorEvalCache (specifically, a DiscriminatorEvalCacheEntry), an internal
// supporting class for Discriminator.  DiscriminatorEvalCache also keeps a reference to the associated Discriminator,
// so there are no retain cycles.
//
// The side benefit is that Taxonomy2 will not have to use DiscriminatorEvalCache for its own translation tables.
class DiscriminatorEvalCacheEntry;

/**
 * Taxonomy 2, described in Magliveras-Leavitt
 * 
 * This construction of a G-invariant function for k-subsets is based upon having a discriminator
 * for subsets of size smaller than k (preferably k - 1).  If this size is t, then the function
 * evaluates the discriminator for every t-subset of the input set, and returns a frequency vector
 * of all the outputs.  (i.e. v[0] is the number of t-subsets of the input set for which the
 * discriminator returns 0, and so on for the rest of v)
 * 
 * Though this works for any t smaller than k, it works best for t = k - 1 as C(k,t) subsets need
 * to be evaluated in this construction.
 */
class Taxonomy2 : public GInvariant {
	typedef std::map<unsigned long, size_t> FrequencyVector;	// sparse "vector" stand-in
	friend class DiscriminatorEvalCacheEntry;					// Only DiscriminatorEvalCacheEntry can create this
public:
	typedef Taxonomy2Evaluator Evaluator;
	
	virtual ~Taxonomy2() {}
	
	bool operator==(const Taxonomy2& rhs) const { return equals(rhs); }
	bool operator!=(const Taxonomy2& rhs) const { return !equals(rhs); }
	bool equals(const GInvariant& rhs) const;
	std::size_t hash() const;
	Evaluator createEvaluator() const;
	
	unsigned long evaluate(const Subset& B) const;
	bool hasCachedResult(const Subset& B) const;
	std::deque<GInvariantEvaluationTask> getDependents(const Subset& B) const;
private:
	Taxonomy2(const Group& _G, const boost::shared_ptr<const Discriminator>& _phi) :
		GInvariant(_G), phi(_phi) {}
	
	boost::shared_ptr<const Discriminator> phi;
};

/* ********************************************************************************************************************* */
// Trait class specializations

template <> struct cachesResults<Taxonomy2> : public boost::true_type {};
template <> struct isDiscriminator<Taxonomy2> : public boost::false_type {};
template <> struct isSizeIndependent<Taxonomy2> : public boost::false_type {};
template <> struct spawnsDependents<Taxonomy2> : public boost::true_type {};
template <> struct isTrivial<Taxonomy2> : public boost::false_type {};
#endif