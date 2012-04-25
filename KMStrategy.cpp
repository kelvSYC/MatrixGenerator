#include <vector>

#include <boost/shared_ptr.hpp>

#include "AnchorSet.h"
#include "Discriminator.h"
#include "KMStrategy.h"

/* *************************************************************************************************************************
 * Taxonomy2Strategy
 ************************************************************************************************************************ */
std::vector<boost::shared_ptr<GInvariant> > Taxonomy2Strategy::createInitialGInvariants(const Group& G, unsigned int k, const TablePrunerData& output) {
	std::vector<boost::shared_ptr<GInvariant> > resultList;
	
	boost::shared_ptr<Discriminator> discriminator = boost::static_pointer_cast<Discriminator>(output.getDiscriminator());
	resultList.push_back(discriminator->getInvariant());
	
	return resultList;
}

boost::shared_ptr<GInvariant> Taxonomy2Strategy::createNewGInvariant(const Group& G, unsigned int k) {
	boost::shared_ptr<AnchorSet> newSet = AnchorSet::buildAnchorSet(G, G.getNumPoints() / 2);
	return newSet;
}

/* *************************************************************************************************************************
 * RecyclerStrategy
 ************************************************************************************************************************ */
std::vector<boost::shared_ptr<GInvariant> > RecyclerStrategy::createInitialGInvariants(const Group& G, unsigned int k, const TablePrunerData& output) {
	std::vector<boost::shared_ptr<GInvariant> > resultList;
	
	boost::shared_ptr<Discriminator> discriminator = boost::static_pointer_cast<Discriminator>(output.getDiscriminator());
	resultList = discriminator->getFunctions();
	
	return resultList;
}

boost::shared_ptr<GInvariant> RecyclerStrategy::createNewGInvariant(const Group& G, unsigned int k) {
	boost::shared_ptr<AnchorSet> newSet = AnchorSet::buildAnchorSet(G, G.getNumPoints() / 2);
	return newSet;
}

