#include "SetImagePruner.h"

#include <deque>

#include <permlib/search/classic/set_image_search.h>

namespace {
	/**
	 * This predicate returns true if the input subset is in the same orbit as the given representative - ie. whether there
	 * exists a permutation which maps the representative to the input subset.
	 */
	class SameOrbitPredicate {
		boost::shared_ptr<const Group> G;
		Subset rep;
	public:
		SameOrbitPredicate(const Group& _G, const Subset& _rep) : G(_G.shared_from_this()), rep(_rep) {}
		
		bool operator()(const Subset& candidate) const {
			// Must construct the search here, as permlib's search classes are not copyable
			permlib::classic::SetImageSearch<PermutationGroup, PermutationGroup::TRANStype> search(G->getGroup(), 0);
			search.construct(rep.begin(), rep.end(), candidate.begin(), candidate.end());
			
			return search.searchCosetRepresentative();
		}
	};
}

SetImagePruner::SetImagePruner(const Group& G, unsigned int k, unsigned long rho, const std::vector<Subset>& orbitReps, const boost::any& prunerData) : Pruner(G, rho, DefaultCandidateGenerator(G.getNumPoints(), orbitReps)) {}

void SetImagePruner::prune() {
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

std::vector<Subset> SetImagePruner::getNewReps() {
	if (!newReps) boost::throw_exception(PrunerNotReady());
	return *newReps;
}

size_t SetImagePruner::getColumn(const Subset& candidate) {
	std::vector<Subset>::iterator findIt = std::find_if(newReps->begin(), newReps->end(), SameOrbitPredicate(*G, candidate));
	
	// findIt should not be newReps->end()
	return std::distance(newReps->begin(), findIt);
}