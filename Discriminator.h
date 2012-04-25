#include <map>

#include "GInvariant.h"

#ifndef DISCRIMINATOR_H
#define DISCRIMINATOR_H

class Taxonomy2;
class DiscriminatorEvaluator;
class DiscriminatorEvalCacheEntry;

typedef std::vector<GInvariant::ptr> GInvariantList;

/**
 * A discriminator for k-subsets is a G-invariant function for which the rank (size of codomain)
 * is the same size as the number of orbits of k-subsets.  The rank of the cartesian product of
 * two G-invariant functions over k-subsets is at least as big as the larger of the two, thus this
 * is a container representing the Cartesian product of a number of G-invariant functions.
 *
 * This object should not be created directly.  Rather, it should be created from the
 * computeNextMatrix function, which guarantees that the discriminator actually discriminates
 * between subsets.
 */
class Discriminator : public GInvariant, public boost::enable_shared_from_this<Discriminator> {
	typedef std::map<std::vector<unsigned long>, unsigned long> LookupTable;
	
	friend class DiscriminatorEvalCacheEntry;
public:
	typedef boost::shared_ptr<Discriminator> ptr;
	typedef DiscriminatorEvaluator Evaluator;
	
	Discriminator(const Group& _G, GInvariantList& _functions, const LookupTable& _lookupTable, const std::map<Subset, unsigned long>& _newCache) :
		GInvariant(_G), functions(_functions), lookupTable(_lookupTable), newCache(_newCache) {}
	virtual ~Discriminator() {}
	
	bool operator==(const Discriminator& rhs) const { return equals(rhs); }
	bool operator!=(const Discriminator& rhs) const { return !equals(rhs); }
	bool equals(const GInvariant& rhs) const;
	std::size_t hash() const;
	Evaluator createEvaluator() const;
	
	/**
	 * Returns the number of possible outputs for this discriminator.  This should be equal to
	 * G.burnside(k) for some value of k.  evaluate(B) should, for all B of size k, return a value from
	 * 0 to numOutputs() - 1.
	 */
	unsigned long numOutputs() const { return lookupTable.size(); }
	GInvariantList getFunctions() const { return functions; }
	const GInvariant::ptr getInvariant() const;
	
	unsigned long evaluate(const Subset& B) const;
	bool hasCachedResult(const Subset& B) const;
	std::deque<GInvariantEvaluationTask> getDependents(const Subset& B) const;
private:
	GInvariantList functions;
	LookupTable lookupTable;					// Should be fully constructed when built
	std::map<Subset, unsigned long> newCache;	// Starting evaluation cache
};

/* ********************************************************************************************************************* */
// Trait class specializations

template <> struct cachesResults<Discriminator> : public boost::true_type {};
template <> struct isDiscriminator<Discriminator> : public boost::true_type {};
template <> struct isSizeIndependent<Discriminator> : public boost::false_type {};
template <> struct spawnsDependents<Discriminator>: public boost::true_type {};
template <> struct isTrivial<Discriminator> : public boost::false_type {};
#endif
