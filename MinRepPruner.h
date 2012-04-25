#include "Pruner.h"

#include <boost/optional.hpp>

#include <permlib/search/orbit_lex_min_search.h>

#ifndef MatrixGenerator_MinRepPruner_h
#define MatrixGenerator_MinRepPruner_h

/**
 * A MinRepPruner computes orbit representatives for k-subsets by repeated application of an algorithm that determines
 * the minimum orbit representative (in lexicographical order) of any k-subset.
 */
class MinRepPruner : public Pruner {
	Subset minRep(const Subset& candidate);
public:
	MinRepPruner(const Group& G, unsigned int k, unsigned long rho, const std::vector<Subset>& orbitReps, const boost::any& prunerData);
	
	void prune();
	
	std::vector<Subset> getNewReps();
	size_t getColumn(const Subset& candidate);
private:
	unsigned long rho;
	
	permlib::OrbitLexMinSearch<PermutationGroup> search;
	
	boost::optional<std::vector<Subset> > newReps;
};

#endif
