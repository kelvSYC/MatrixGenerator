#include "ExplicitPruner.h"

#include <algorithm>
#include <deque>
#include <iterator>

#include <boost/bind.hpp>
#include <boost/throw_exception.hpp>

namespace {
	class SameOrbitPredicate {
		boost::shared_ptr<const Group> G;
		Subset rep;
	public:
		SameOrbitPredicate(const Group& _G, const Subset& _rep) : G(_G.shared_from_this()), rep(_rep) {}
		
		bool operator()(const Subset& candidate) {
			for (GroupElementIterator it = G->elementsBegin(); it != G->elementsEnd(); ++it) {
				// Compute image of candidate under *it
				Subset image;
				std::transform(candidate.begin(), candidate.end(), std::inserter(image, image.begin()), boost::bind(&Permutation::at, *it, _1));
				
				if (rep == image) return true;
			}
			return false;
		}
	};
}

ExplicitPruner::ExplicitPruner(const Group& G, unsigned int k, unsigned long rho, const std::vector<Subset>& orbitReps, const boost::any& prunerData) : Pruner(G, rho, DefaultCandidateGenerator(G.getNumPoints(), orbitReps)), rho(rho) {}

void ExplicitPruner::prune() {
	newReps = std::vector<Subset>();
	newReps->reserve(rho);
	const std::vector<Subset>& candidates = getCandidates();
	
	std::deque<Subset> remainingCandidates(candidates.begin(), candidates.end());
	
	while (!remainingCandidates.empty()) {
		Subset rep = remainingCandidates.front();
		remainingCandidates.pop_front();
		newReps->push_back(rep);
		
		// Erase everything in remainingCandidates in the same orbit
		remainingCandidates.erase(
			std::remove_if(remainingCandidates.begin(), remainingCandidates.end(), SameOrbitPredicate(*G, rep)),
			remainingCandidates.end());
	}
}

std::vector<Subset> ExplicitPruner::getNewReps() {
	if (!newReps) boost::throw_exception(PrunerNotReady());
	return *newReps;
}

size_t ExplicitPruner::getColumn(const Subset& candidate) {
	if (!newReps) boost::throw_exception(PrunerNotReady());
	
	std::vector<Subset>::iterator findIt = std::find_if(newReps->begin(), newReps->end(), SameOrbitPredicate(*G, candidate));
	
	// assert findIt != newReps->end()
	return std::distance(newReps->begin(), findIt);
}