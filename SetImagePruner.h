#include "Pruner.h"

#include <boost/optional.hpp>

#ifndef MatrixGenerator_SetImagePruner_h
#define MatrixGenerator_SetImagePruner_h

/**
 * A SetImagePruner computes orbit representatives for k-subsets by taking candidates and finding whether or not there is
 * a permutation in G that maps one candidate to another candidate.  A candidate is eliminated if there is a permutation
 * mapping it to a "smaller" candidate.
 */
class SetImagePruner : public Pruner {
public:
	SetImagePruner(const Group& G, unsigned int k, unsigned long rho, const std::vector<Subset>& orbitReps, const boost::any& prunerData);
	
	void prune();
	
	std::vector<Subset> getNewReps();
	size_t getColumn(const Subset& candidate);
private:
	boost::optional<std::vector<Subset> > newReps;
};

#endif
