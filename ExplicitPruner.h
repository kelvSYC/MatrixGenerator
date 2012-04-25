#include "Pruner.h"

#include <boost/optional.hpp>

#ifndef MatrixGenerator_ExplicitPruner_h
#define MatrixGenerator_ExplicitPruner_h

/**
 * An ExplicitPruner computes orbit representatives for k-subsets by taking candidates and explicitly computing the orbits
 * to eliminate everything else.
 */
class ExplicitPruner : public Pruner {
public:
	ExplicitPruner(const Group& G, unsigned int k, unsigned long rho, const std::vector<Subset>& orbitReps, const boost::any& prunerData);
	
	void prune();
	
	std::vector<Subset> getNewReps();
	size_t getColumn(const Subset& candidate);
private:
	unsigned long rho;
	
	boost::optional<std::vector<Subset> > newReps;
};

#endif
