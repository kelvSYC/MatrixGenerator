#include "Pruner.h"
#include "utils.h"

std::vector<Subset> DefaultCandidateGenerator::generateCandidates() const {
	std::set<Subset> labels;
	
	for (std::vector<Subset>::const_iterator it = getOldReps().begin(); it != getOldReps().end(); ++it) {
		Subset Y = xMinus(getNumPoints(), *it);
		
		// Iterate only through elements in Y greater than the last element in *it
		Subset::const_iterator iterStart = Y.upper_bound(*(it->rbegin()));
		for (Subset::const_iterator jt = iterStart; jt != Y.end(); jt++) {
			Subset label(it->begin(), it->end());		// *it ...
			label.insert(*jt);							// ... union {*jt}
			
			labels.insert(label);
		}
	}
	
	return std::vector<Subset>(labels.begin(), labels.end());
}

std::vector<Subset> FullCandidateGenerator::generateCandidates() const {
	std::set<Subset> labels;
	
	for (std::vector<Subset>::const_iterator it = getOldReps().begin(); it != getOldReps().end(); ++it) {
		Subset Y = xMinus(getNumPoints(), *it);
		for (Subset::const_iterator jt = Y.begin(); jt != Y.end(); ++jt) {
			Subset label(it->begin(), it->end());		// *it ...
			label.insert(*jt);							// ... union {*jt}
			
			labels.insert(label);
		}
	}
	
	return std::vector<Subset>(labels.begin(), labels.end());
}
