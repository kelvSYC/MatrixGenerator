#include "GInvariant.h"

#ifndef TAXONOMY1_H
#define TAXONOMY1_H

class Taxonomy1Evaluator;

typedef std::set<Subset> Partition;

/**
 * Taxonomy 1, as defined by Magliveras-Leavitt.
 *
 * Each permutation of the group G is associated with a Taxonomy1, though the efficiency differs
 * between permutations (in particular, the identity element should be the function with the worst
 * efficiency of them all).
 *
 * The construction itself is centered about the disjoint cycle representation of the permutation,
 * when treated as a partition of X = {1, .., v}, for which the orbit is then calculated; the orbit
 * of a partition is the set of partitions created by applying each permutation to each subset
 * therein (basically akin to "interconnected small anchor sets").
 *
 * The input subset is transformed into a frequency vector for each partition in the orbit exactly
 * as in anchor set evaluation, and the resulting frequency vectors are then accumulated in a
 * "frequency vector of frequency vectors" to produce the result.
 */
class Taxonomy1 : public GInvariant {
	typedef std::vector<Subset::size_type> FrequencyVector;
	typedef std::map<std::multiset<FrequencyVector>, unsigned long> LookupTable;
public:
	typedef Taxonomy1Evaluator Evaluator;
	
	Taxonomy1(const Group& _G, const Permutation& _basePerm);
	virtual ~Taxonomy1() {}
	
	const Permutation& getBasePerm() const { return basePerm; }
	
	bool operator==(const Taxonomy1& rhs) const { return equals(rhs); }
	bool operator!=(const Taxonomy1& rhs) const { return !equals(rhs); }
	bool equals(const GInvariant& rhs) const;
	std::size_t hash() const;
	Evaluator createEvaluator() const;
	
	unsigned long evaluate(const Subset& B) const;
	bool hasCachedResult(const Subset& B) const;
private:
	Permutation basePerm;
	
	std::set<Partition> pOrbit;
};

/**
 * Weak ordering for Taxonomy1 suitable for use with containers like std::map.  Taxonomy1 should not
 * be less-than-comparable in general.
 */
struct Taxonomy1WeakOrdering {
	bool operator()(const Taxonomy1& lhs, const Taxonomy1& rhs) const;
};

/* ********************************************************************************************************************* */
// Trait class specializations

template <> struct cachesResults<Taxonomy1> : public boost::true_type {};
template <> struct isDiscriminator<Taxonomy1> : public boost::false_type {};
template <> struct isSizeIndependent<Taxonomy1> : public boost::true_type {};
template <> struct spawnsDependents<Taxonomy1> : public boost::false_type {};
template <> struct isTrivial<Taxonomy1> : public boost::false_type {};
#endif