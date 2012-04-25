#include <map>

#include <boost/unordered_map.hpp>
#include <boost/type_traits/integral_constant.hpp>

#ifndef ANCHORSET_H
#define ANCHORSET_H

#include "GInvariant.h"

class AnchorSetEvaluator;

/**
 * Anchor set construction, described in Magliveras-Leavitt.
 * 
 * This construction is centered about the anchor set, an arbitrary fixed subset of X = {1, .., v}
 * for which the orbit is calculated.  The input subset is then transformed into a frequency
 * vector, whose ith entry is the number of images which intersect B in exactly i points.
 */
class AnchorSet : public GInvariant {
public:
	typedef boost::shared_ptr<AnchorSet> ptr;
	typedef AnchorSetEvaluator Evaluator;
	
	virtual ~AnchorSet() {}
	
	static ptr buildAnchorSet(const Group& G, size_t size);
	
	const Subset& getAnchorSet() const { return anchorSet; }
	
	bool operator==(const AnchorSet& rhs) const { return equals(rhs); }
	bool operator!=(const AnchorSet& rhs) const { return !equals(rhs); }
	bool equals(const GInvariant& rhs) const;
	std::size_t hash() const;
	Evaluator createEvaluator() const;
	
	unsigned long evaluate(const Subset& B) const;
	bool hasCachedResult(const Subset& B) const;
private:
	AnchorSet(const Group& G, const Subset& anchorset);	// Must go through factory
	
	Subset anchorSet;							// The anchor set, for comparison purposes
	// Permutation does not have operator<(), so we must use unordered maps for the image
	// set.  Replace with std::unordered_map with C++0x.
	boost::unordered_map<Permutation, Subset> imageSet;
};

/**
 * Weak ordering for AnchorSet that will make AnchorSet work with containers such as std::map.
 * AnchorSets should not normally be ordered in general.
 */
struct AnchorSetWeakOrdering {
	bool operator()(const AnchorSet& lhs, const AnchorSet& rhs) const;
};

/* ********************************************************************************************************************* */
// Trait class specializations

template <> struct cachesResults<AnchorSet> : public boost::true_type {};
template <> struct isDiscriminator<AnchorSet> : public boost::false_type {};
template <> struct isSizeIndependent<AnchorSet> : public boost::true_type {};
template <> struct spawnsDependents<AnchorSet>: public boost::false_type {};
template <> struct isTrivial<AnchorSet> : public boost::false_type {};

#endif