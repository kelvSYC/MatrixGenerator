#include "utils.h"

#include <boost/iterator/counting_iterator.hpp>

// Slightly pollute permlib namespace for this...
namespace permlib {
	std::size_t hash_value(const Permutation& perm) {
		// Permutation is backed by std::vector, but with fewer public accessors.
		// So we just copy hash_value(std::vector)
		std::size_t seed = 0;
		for (int i = 0; i < perm.size(); ++i) {
			boost::hash_combine(seed, perm.at(i));
		}
		return seed;
	}
}

/* FUNCTIONS THAT ARE SYNTACTIC SUGAR ********************************************************** */

/**
 * Creates the set {0, .., v - 1}, which is how the set X = {1, .., v} is represented internally.
 */
Subset generateX(unsigned int v) {
	using boost::counting_iterator;
	
	Subset result;
	std::copy(counting_iterator<unsigned long>(0), counting_iterator<unsigned long>(v),
		std::inserter(result, result.begin()));
	
	return result;
}

/**
 * Creates the set X - B, where B is a subset of X and X = {1, .., v}.  As with other X-functions,
 * the set X is represented by {0, .., v - 1}, due to permutation's internal representation.
 */
Subset xMinus(unsigned int v, const Subset& B) {
	using boost::counting_iterator;
	
	Subset result;
	std::set_difference(counting_iterator<int>(0), counting_iterator<int>(v),
		B.begin(), B.end(),
		std::inserter(result, result.begin()));
		
	return result;
}

/* FUNCTIONS THAT ARE NOT SYNTACTIC SUGAR ****************************************************** */
/**
 * Returns a permutation as a list of disjoint cycles.
 * In PermLib, a Permutation is stored internally in "list notation", so this is needed.
 */
std::vector<Cycle> permutationCycles(const Permutation& g, int v) {
	// Permutation stores {1, .., v} as {0, .., v - 1}
	Subset pointsRemaining = generateX(v);
	
	std::vector<Cycle> cycles;
	while (!pointsRemaining.empty()) {
		Cycle cycle;
		
		unsigned long firstPoint = *(pointsRemaining.begin());
		unsigned long nextPoint = firstPoint;
		do {
			cycle.push_back(nextPoint);
			pointsRemaining.erase(pointsRemaining.find(nextPoint));
			
			// Next point in cycle
			nextPoint = g.at(nextPoint);
		} while (nextPoint != firstPoint);
		
		cycles.push_back(cycle);
	}
	
	return cycles;
}

/**
 * Partitions the input integer k.
 *
 * This particular implementation is particularly inefficient for large values of k (as the list
 * of partitions grows large), so it should be replaced with one based on a forward iterator.
 * In practice, the inputs will tend to be on the small size...
 */
std::set<std::multiset<unsigned int> > partition(unsigned int k) {
	if (k == 1) {
		std::multiset<unsigned int> singleton;
		singleton.insert(1);
		
		std::set<std::multiset<unsigned int> > result;
		result.insert(singleton);
		return result;
	} else {
		std::set<std::multiset<unsigned int> > result;
		for (int i = 1; i < k; i++) {
			// Partition k - i and append i to each result therein
			std::set<std::multiset<unsigned int> > smaller = partition(k - i);
			
			for (std::set<std::multiset<unsigned int> >::iterator it = smaller.begin(); it != smaller.end(); it++) {
				std::multiset<unsigned int> ms(*it);	// Copy the multiset
				ms.insert(i);
				result.insert(ms);
			}
		}
		
		// Finally, add the singleton k itself
		std::multiset<unsigned int> singleton;
		singleton.insert(k);
		result.insert(singleton);
		
		return result;
	}
}

/**
 * Computes the binomial coefficient.
 *
 * Note that we avoid boost::math::binomial_coefficient() because it's not recommended for
 * integer return types (it uses factorials and beta function).
 */
unsigned int combinat(unsigned int n, unsigned int k) {
	if (k > n - k) k = n - k;		// Symmetry of the binomial coefficient
	int result = 1;
	for (int i = 0; i < k; i++) {
		result *= n - i;
		result /= i + 1;			// Avoids fractions
	}
	
	return result;
}

bool PermutationWeakOrdering::operator()(const Permutation& perm1, const Permutation& perm2) const {
	int pos = 0;
	for (; pos < perm1.size(); pos++) {
		if (pos == perm2.size() || perm2.at(pos) < perm1.at(pos)) return false;
		else if (perm1.at(pos) < perm2.at(pos)) return true;
	}
	return (pos != perm2.size());
}