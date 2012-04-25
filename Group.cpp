#include "Cache.h"
#include "Group.h"

#include <algorithm>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/mem_fn.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/functional/hash.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>

#include <permlib/construct/schreier_sims_construction.h>

/* ********************************************************************************************************** */
class GroupBurnsideEvaluator {
	const Group& G;
public:
	GroupBurnsideEvaluator(const Group& _G) : G(_G) {}
	
	unsigned long operator()(unsigned int k) const {
		// Partition k and group by number
		std::set<std::multiset<unsigned int> > partitions = partition(k);
		std::vector<std::vector<unsigned int> > partitionTallies;
		for (std::set<std::multiset<unsigned int> >::const_iterator it = partitions.begin(); it != partitions.end(); it++) {
			std::vector<unsigned int> tally(k);
			for (unsigned int i = 0; i < k; i++) {
				tally[i] = it->count(i + 1);
			}
			partitionTallies.push_back(tally);
		}
		
		unsigned long s = 0;
		
		for (GroupElementIterator it = G.elementsBegin(); it != G.elementsEnd(); ++it) {
			Permutation g = *it;
			std::vector<Cycle> cycles = permutationCycles(g, G.getNumPoints());
			
			// Get list of cycle lengths, but only for cycles of length at most k
			std::vector<unsigned int> cycleLengths(k);
			for (std::vector<Cycle>::const_iterator jt = cycles.begin(); jt != cycles.end(); jt++) {
				if (jt->size() <= k) cycleLengths[jt->size() - 1]++;
			}
			
			for (std::vector<std::vector<unsigned int> >::const_iterator jt = partitionTallies.begin(); jt != partitionTallies.end(); jt++) {
				unsigned long intermediate = 1;
				for (int i = 0; i < k; i++) {
					// Do binomial coefficients pairwise
					intermediate *= combinat(cycleLengths[i], (*jt)[i]);
				}
				s += intermediate;
			}
		}
		
		return s / G.order();
	}
};

/* ********************************************************************************************************** */
class GroupBurnsideLookupTable : public StdMapCache<unsigned int, unsigned long, GroupBurnsideEvaluator>::type {
	typedef StdMapCache<unsigned int, unsigned long, GroupBurnsideEvaluator>::type super_type;
public:
	GroupBurnsideLookupTable(const Group& G) : super_type(GroupBurnsideEvaluator(G)) {}
};

/* ********************************************************************************************************** */
// This delegate class is just a placeholder until C++03 can be fully abandoned and the default insert delegates
// can take on arguments...
struct GroupBurnsideCacheDelegate {
	boost::shared_ptr<GroupBurnsideLookupTable> operator()(const Group& G) const {
		return boost::shared_ptr<GroupBurnsideLookupTable>(new GroupBurnsideLookupTable(G));
	}
};

/**
 * Burnside cache for Group.  This caches the result of Group::burnside() to avoid recalculating it.
 * 
 * This is implemented as a "classic singleton", so is guaranteed thread-safe under C++11, but under C++03
 * this is limited to single-threaded operation.  It should be thread-safe under C++03 on gcc and clang, the
 * two compilers used in development.
 */
class GroupBurnsideCache : public boost::noncopyable, public HeapValueStdMapCache<Group, GroupBurnsideLookupTable, GroupBurnsideCacheDelegate, GroupWeakOrdering>::type {
	// boost::noncopyable also implicitly deletes move constructor
public:
	static GroupBurnsideCache& getInstance() {
		static GroupBurnsideCache instance;
		return instance;
	}
	
private:
	GroupBurnsideCache() {}
};

/* ********************************************************************************************************** */
// GroupWeakOrdering methods

bool GroupWeakOrdering::operator()(const Group& lhs, const Group& rhs) const {
	// PermutationGroup does not implement operator<(), and permlib::bsgs_core does not expose their "group ID"
	// which their operator==() is based off of, so we have to do this.
	const PermutationGroup& Gl = lhs.getGroup();
	const PermutationGroup& Gr = rhs.getGroup();
	
	if (Gl.B < Gr.B) return true;	// PermutationGroup::B is a std::vector of integral type
	else {
		// PermutationGroup::S is a PermutationGroup:PERMlist, which is a std::list of pointers
		// So we have to dereference everything first to compare the SGSes
		typedef boost::indirect_iterator<PermutationGroup::PERMlist::const_iterator> ListValueIterator;
		
		return std::lexicographical_compare(
			ListValueIterator(Gl.S.begin()), ListValueIterator(Gl.S.end()),
			ListValueIterator(Gr.S.begin()), ListValueIterator(Gr.S.end()),
			PermutationWeakOrdering());
	}
}

/* ********************************************************************************************************** */
// GroupElementIterator methods

GroupElementIterator::GroupElementIterator(const boost::shared_ptr<Group>& G_) :
	G(G_), begins(), ends(), state() {
	const std::vector<Transversal>& transversals = G->getGroup().U;
		std::transform(transversals.begin(), transversals.end(), std::inserter(begins, begins.begin()), boost::mem_fn(&Transversal::begin));
		std::transform(transversals.begin(), transversals.end(), std::inserter(ends, ends.begin()), boost::mem_fn(&Transversal::end));
}

/**
 * Dereferences the iterator.
 */
Permutation GroupElementIterator::dereference() const {
	const std::vector<Transversal>& transversals = G->getGroup().U;
	Permutation g(G->getNumPoints());
	
	// The next permutation is the product of all the transversal elements pointed to by the iterator state
	// The multiplication MUST be done in reverse order (we could do it forwards, but we must invert everything...
	typedef TransversalIterators::const_reverse_iterator StateIterator;
	typedef std::vector<Transversal>::const_reverse_iterator TransversalsIterator;
	typedef boost::zip_iterator<boost::tuple<StateIterator, TransversalsIterator> > Iterator;
	for (Iterator it = Iterator(boost::make_tuple(state.rbegin(), transversals.rbegin())); it != Iterator(boost::make_tuple(state.rend(), transversals.rend())); ++it) {
		// Transversal::at() returns a pointer to a Permutation for which we have to take ownership of, so we have to
		// wrap it in a scoped_ptr for safe deletion.
		boost::scoped_ptr<Permutation> rhs(it->get<1>().at(*it->get<0>()));
		g *= *rhs;
	}
	return g;
}

GroupElementIterator& GroupElementIterator::operator++() {
	increment();
	return *this;
}

GroupElementIterator GroupElementIterator::operator++(int) {
	GroupElementIterator tmp(*this);
	increment();
	return tmp;
}

/**
 * Increments the group element iterator.
 */
void GroupElementIterator::increment() {
	for (size_t i = 0; i < state.size();) {
		state[i]++;
		if (state[i] == ends[i]) {			// Iterator is about to roll over
			if (i == state.size() - 1) {
				return;						// The last iterator rolled over, so we are "one past the end"
			} else {
				state[i] = begins[i];		// Reset iterator
				i++;						// Move on to the next one
			}
		} else {
			return;
		}
	}
}

/* ********************************************************************************************************** */
// Group methods

/**
 * Constructs a new group.
 * 
 * Precondition: generators is a a list of disjoint cycles.  (Not a real precondition, but it's
 * easier this way.)
 *
 * @param v Number of points - ie. the size of X = {1, .., v}
 * @param generators The generators of the group.
 */
Group::Group(unsigned int _v, const std::list<Cycles>& _generators) :
	v(_v), generators(_generators), generatorPermutations(), G(_v) {
	// Create the generator Permutations
	for (std::list<Cycles>::const_iterator it = generators.begin(); it != generators.end(); it++) {
		// First, we have to convert our generator into a string
		// Why we have to do this when Permutation's constructor will just take it apart...
		std::list<std::string> permStrings;
		for (Cycles::const_iterator jt = it->begin(); jt != it->end(); jt++) {	// for each cycle in generator
			std::list<std::string> cycleStrings;
			
			// If only C++03 has to_string in <string>...
			std::transform(jt->begin(), jt->end(),
						   std::inserter(cycleStrings, cycleStrings.begin()), boost::lexical_cast<std::string, unsigned long>);
			std::string cycleAsString(boost::join(cycleStrings, " "));
			permStrings.push_back(cycleAsString);
		}
		std::string permAsString(boost::join(permStrings, ", "));
		
		// SchreierSimsConstruction::construct() requires iterators over Permutation::ptr, not Permutation
		generatorPermutations.push_back(Permutation::ptr(new Permutation(v, permAsString)));
	}
	
	// Construct the PermutationGroup itself
	permlib::SchreierSimsConstruction<Permutation, Transversal> construction(v);
	G = construction.construct(generatorPermutations.begin(), generatorPermutations.end());
}

bool Group::operator==(const Group& rhs) const {
	// We don't really care about the generators that created the group, we only care about how it is represented.
	return G == rhs.G;
}

GroupElementIterator Group::elementsBegin() const {
	GroupElementIterator it(boost::const_pointer_cast<Group>(shared_from_this()));
	it.state = it.begins;
	return it;
}

GroupElementIterator Group::elementsEnd() const {
	GroupElementIterator it(boost::const_pointer_cast<Group>(shared_from_this()));
	
	// The state should be "one past the end"...
	it.state = it.begins;							// so the state should be begin() for all but the last iterator...
	it.state.back() = it.ends.back();				// ... and end() for the last one.
	return it;
}

/**
 * Computes the number of orbits of k-subsets of X = {1, ..,v}.
 */
unsigned long Group::burnside(unsigned int k) const {
	GroupBurnsideLookupTable& table = GroupBurnsideCache::getInstance().query(*this);
	
	return table.query(k);
}

std::size_t hash_value(const Group& G) {
	std::size_t seed = 0;
	boost::hash_combine(seed, G.getGroup().B);
	
	// Hash the actual stuff in the SGS, not the pointers to them
	typedef boost::indirect_iterator<PermutationGroup::PERMlist::const_iterator> Iterator;
	boost::hash_range(seed, Iterator(G.getGroup().S.begin()), Iterator(G.getGroup().S.end()));
	
	return seed;
}