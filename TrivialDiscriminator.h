#include "GInvariant.h"

#ifndef TrivialDiscriminator_h
#define TrivialDiscriminator_h

/**
 * When the evaluation table has only one column, then it's pretty pointless to add rows to the table: all GInvariants
 * on all inputs will just return one value.  To this end, this is a simple GInvariant that does this, without the
 * overhead of evaluation caches and stuff.
 */
struct TrivialDiscriminator : public GInvariant {
	TrivialDiscriminator(const Group& G) : GInvariant(G) {}
	virtual ~TrivialDiscriminator() {}
	
	bool equals(const GInvariant& rhs) const;
	std::size_t hash() const;
	
	/**
	 * Evaluates this G-invariant function on the input subset.  As a trivial discriminator, this just returns a
	 * constant value.
	 */
	unsigned long evaluate(const Subset&) const { return 0; }
	
	/**
	 * Returns whether the result of the function evaluated over the input subset has previously been calculated.
	 * Since this is a constant function, it should always be "previously calculated".
	 */
	bool hasCachedResult(const Subset&) const { return true; }
};

/**
 * Weak Ordering functor for TrivialDiscriminator, used to put TrivialDiscriminators as keys in ordered associative
 * containers.
 */
struct TrivialDiscriminatorWeakOrdering {
	bool operator()(const TrivialDiscriminator& lhs, const TrivialDiscriminator& rhs) const;
};

/* ********************************************************************************************************************* */
// Trait class specializations

template <> struct cachesResults<TrivialDiscriminator> : public boost::false_type {};
template <> struct isDiscriminator<TrivialDiscriminator> : public boost::true_type {};
template <> struct isSizeIndependent<TrivialDiscriminator> : public boost::true_type {};	// If only on a technicality...
template <> struct spawnsDependents<TrivialDiscriminator> : public boost::false_type {};
template <> struct isTrivial<TrivialDiscriminator> : public boost::true_type {};
#endif
