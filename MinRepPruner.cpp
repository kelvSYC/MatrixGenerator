#include "MinRepPruner.h"

#include <algorithm>

#include <boost/dynamic_bitset.hpp>
#include <boost/throw_exception.hpp>

MinRepPruner::MinRepPruner(const Group& G, unsigned int k, unsigned long _rho, const std::vector<Subset>& orbitReps, const boost::any& prunerData) : Pruner(G, _rho, DefaultCandidateGenerator(G.getNumPoints(), orbitReps)), rho(_rho), search(G.getGroup()) {}

Subset MinRepPruner::minRep(const Subset& candidate) {
	typedef boost::dynamic_bitset<unsigned long> dset;
	
	dset candidateBitset(G->getNumPoints());
	for (Subset::const_iterator it = candidate.begin(); it != candidate.end(); ++it) {
		candidateBitset[*it] = true;
	}
	
	dset resultBitset = search.lexMin(candidateBitset);
	
	Subset result;
	for (dset::size_type i = resultBitset.find_first(); i != dset::npos; i = resultBitset.find_next(i)) {
		result.insert(i);
	}
	return result;
}

void MinRepPruner::prune() {
	newReps = std::vector<Subset>();
	newReps->reserve(rho);
	
	const std::vector<Subset>& candidates = getCandidates();
	for (std::vector<Subset>::const_iterator it = candidates.begin(); (newReps->size() != rho) && (it != candidates.end()); ++it) {
		// Short-circuit if we found all the reps
		if (*it == minRep(*it)) {
			newReps->push_back(*it);
		}
	}
}

size_t MinRepPruner::getColumn(const Subset& candidate) {
	if (!newReps) boost::throw_exception(PrunerNotReady());
	Subset minimumRep = minRep(candidate);
	
	std::vector<Subset>::iterator it = std::find(newReps->begin(), newReps->end(), minimumRep);
	return std::distance(newReps->begin(), it);
}

std::vector<Subset> MinRepPruner::getNewReps() {
	if (!newReps) boost::throw_exception(PrunerNotReady());
	return *newReps;
}