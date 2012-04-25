#include <vector>

#include <boost/shared_ptr.hpp>

#include "TablePruner.h"

#ifndef MatrixGenerator_KMStrategy_h
#define MatrixGenerator_KMStrategy_h

class GInvariant;

/**
 * A KMStrategy is used by a TablePruner in determining the algorithm specifics with regards to which GInvariants to use in
 * various situations.
 */
struct KMStrategy {
	/**
	 * Constructs the initial list of GInvariants to be fed into the TablePruner from the TablePrunerData in the previous
	 * iteration.
	 *
	 * Precondition: output.isTrivial() == false, so output.getDiscriminator() can be safely cast to
	 * boost::shared_ptr<Discriminator>
	 *
	 * Postcondition: If GInvariantType is the true type of any GInvariant in the returned list, then it is the case that
	 * isDiscriminator<GInvariantType>::value == false and isTrivial<GInvariantType>::value == false
	 */
	virtual std::vector<boost::shared_ptr<GInvariant> > createInitialGInvariants(const Group& G, unsigned int k, const TablePrunerData& output) = 0;
	
	/**
	 * Constructs a new GInvariant to be fed into the TablePruner when all of the GInvariant functions in the initial list
	 * have been fed and a Discriminator has yet to be found.
	 *
	 * Postcondition: If GInvariantType is the returned GInvariant's actual type, then it is the case that
	 * isDiscriminator<GInvariantType>::value == false and isTrivial<GInvariantType>::value == false
	 */
	virtual boost::shared_ptr<GInvariant> createNewGInvariant(const Group& G, unsigned int k) = 0;
};

/**
 * Concrete implementation of the KMStrategy interface, which uses a Taxonomy2 as its only initial candidate, and resorts
 * to random AnchorSets in the event that the Taxonomy2 fails to discriminate.
 */
struct Taxonomy2Strategy : public KMStrategy {
	std::vector<boost::shared_ptr<GInvariant> > createInitialGInvariants(const Group& G, unsigned int k, const TablePrunerData& output);
	boost::shared_ptr<GInvariant> createNewGInvariant(const Group& G, unsigned int k);
};

/**
 * Concrete implementation of the KMStrategy interface, which reuses the functions used bo build the Discriminator as its
 * initial candidates, and resorts to random AnchorSets in the event that the list fails to discriminate.  Note that this
 * exclusively uses the size-independent AnchorSet to do all its work.
 */
struct RecyclerStrategy : public KMStrategy {
	std::vector<boost::shared_ptr<GInvariant> > createInitialGInvariants(const Group& G, unsigned int k, const TablePrunerData& output);
	boost::shared_ptr<GInvariant> createNewGInvariant(const Group& G, unsigned int k);
};
#endif
