#include <algorithm>
#include <iterator>

#include "ExplicitPruner.h"
#include "KMBuilder.h"
#include "KMStrategy.h"
#include "MinRepPruner.h"
#include "SetImagePruner.h"

KMBuilder::KMBuilder(const Group& G_, unsigned int k, const std::vector<Subset>& orbitReps_, const boost::any& _prunerData) :
G(G_.shared_from_this()), orbitReps(orbitReps_), rho(G->burnside(k)), timer(std::cerr, 2) {
	if (rho == 1) {
		std::cerr << "Iteration for k = " << k << " is trivial" << std::endl;
		
		// If this is the case, then we know the following:
		// * orbitReps == {1, ..., k - 1}, and the new orbitReps should be {1, ..., k}
		// * A is a 1x1 matrix, whose entry is G->getNumPoints() - k
		// So, since we basically have everything, we are ready
		
//		ready = true;
	} else {
		// Prepare the pruner
		boost::shared_ptr<KMStrategy> strategy = boost::shared_ptr<RecyclerStrategy>(new RecyclerStrategy());
		pruner = boost::shared_ptr<Pruner>(new TablePruner(*G, k, rho, orbitReps, strategy, _prunerData));
//		pruner = boost::shared_ptr<Pruner>(new ExplicitPruner(*G, k, rho, orbitReps, _prunerData));
//		pruner = boost::shared_ptr<Pruner>(new MinRepPruner(*G, k, rho, orbitReps, _prunerData));
//		pruner = boost::shared_ptr<Pruner>(new SetImagePruner(*G, k, rho, orbitReps, _prunerData));
	}
}

/**
 * Builds the Discriminator output.  It does so by running the pruner and then constructing the next
 * Kramer-Mesner matrix.
 */
KMBuilderOutput KMBuilder::build() {
	std::vector<Subset> newReps;
	newReps.reserve(rho);
	Matrix A(boost::extents[orbitReps.size()][rho]);
	
	if (rho == 1) {
		// We have the trivial case
		
		unsigned int k = *(orbitReps[0].rbegin()) + 2;	// We didn't store k, but we can reconstruct it
		newReps.push_back(generateX(k));
		A[0][0] = G->getNumPoints() - k;
		
		return KMBuilderOutput(newReps, A);
	} else {
		pruner->prune();
		newReps = pruner->getNewReps();
		
		// TODO - find a way to generate the KM matrix without this
		FullCandidateGenerator g(G->getNumPoints(), orbitReps);
		std::vector<Subset> candidates = g.generateCandidates();
		
		for (std::vector<Subset>::iterator it = candidates.begin(); it != candidates.end(); ++it) {
			// There is at least one (k-1)-representative generating this candidate.  We need to find all such sets
			for (Subset::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
				Subset generator(*it);		// *it...
				generator.erase(*jt);		// ... less one element
				
				std::vector<Subset>::iterator genIt = std::find(orbitReps.begin(), orbitReps.end(), generator);
				if (genIt != orbitReps.end()) {
					// If in fact generator is a (k-1)-representative, then we can increment the appropriate cell
					typedef std::iterator_traits<std::vector<Subset>::iterator>::difference_type index_t;
					index_t row = std::distance(orbitReps.begin(), genIt);
					index_t col = pruner->getColumn(*it);		// TODO - find this value without using the pruner
					
					A[row][col]++;
				}
			}
		}
		
		std::cerr << "Iteration for k = " << candidates.front().size() << " complete" << std::endl;
		return KMBuilderOutput(newReps, A, pruner->getNewData());
	}
}