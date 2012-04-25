#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

#include <boost/functional/hash.hpp>
#include <permlib/permutation.h>
#include <permlib/bsgs.h>
#include <permlib/transversal/schreier_tree_transversal.h>

#ifndef UTILS_H
#define UTILS_H

using permlib::BSGS;
using permlib::Permutation;
using permlib::SchreierTreeTransversal;

typedef std::set<unsigned long> Subset;
typedef std::list<unsigned long> Cycle;
typedef std::vector<Cycle> Cycles;

typedef SchreierTreeTransversal<Permutation> Transversal;
typedef BSGS<Permutation, Transversal> PermutationGroup;

template<class T>
std::set<T> makeSingletonSet(T& item);
template<class T>
std::set<T> setIntersection(const std::set<T>& A, const std::set<T>& B);
template<class T>
std::set<T> setDifference(const std::set<T>& A, const std::set<T>& B);

Subset generateX(unsigned int v);
Subset xMinus(unsigned int v, const Subset& B);

// Slightly pollute the permlib namespace for this...
namespace permlib {
	/**
	 * Hash function overload for boost::hash for Permutation
	 */
	std::size_t hash_value(const Permutation& perm);
}

std::vector<Cycle> permutationCycles(const Permutation& g, int v);
std::set<std::multiset<unsigned int> > partition(unsigned int k);
unsigned int combinat(unsigned int n, unsigned int k);

inline void printSubset(const Subset& B) {
	for (Subset::const_iterator it = B.begin(); it != B.end(); it++) {
		std::cerr << *it << " ";
	}
	std::cerr << std::endl;
}

// Permutation has an embedded std::vector, but does not expose most interfaces; this is a
// weak ordering that can be used for Permutation, based on std::lexicographical_compare
struct PermutationWeakOrdering {
	bool operator()(const Permutation& perm1, const Permutation& perm2) const;
};

/* TEMPLATE FUNCTIONS ************************************************** */

/**
 * Makes a new set with only one member.  The returned set can be modified.
 */
template <class T>
std::set<T> makeSingletonSet(T& item) {
	std::set<T> singleton;
	singleton.insert(item);
	
	return singleton;
}

/**
 * Creates a new set consisting of the intersection of two sets.
 */
template <class T>
std::set<T> setIntersection(const std::set<T>& A, const std::set<T>& B) {
	std::set<T> result;
	std::set_intersection(A.begin(), A.end(), B.begin(), B.end(),
						  std::inserter(result, result.begin()));
	
	return result;
}

/**
 * Creates a new set consisting of the set difference between two sets.
 */
template <class T>
std::set<T> setDifference(const std::set<T>& A, const std::set<T>& B) {
	std::set<T> result;
	std::set_difference(A.begin(), A.end(), B.begin(), B.end(),
						std::inserter(result, result.begin()));
	
	return result;
}
#endif