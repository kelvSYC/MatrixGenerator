#include "utils.h"

#include <list>

#include <boost/enable_shared_from_this.hpp>
#include <permlib/transversal/orbit_set.h>

#ifndef GROUP_H
#define GROUP_H

using permlib::OrbitSet;

class GroupElementIterator;		// Forward declare for Group

/**
 * A class representing an permutation group.  Mainly present for convenience, and the fact that
 * the group generation process doesn't keep the original list of generators...
 */
class Group : public boost::enable_shared_from_this<Group> {
public:
	Group(unsigned int v, const std::list<Cycles>& generators);
	
	const std::list<Cycles>& getGenerators() const { return generators; }
	const PermutationGroup& getGroup() const { return G; }
	
	unsigned int getNumPoints() const { return v; }
	boost::uint64_t order() const { return G.order(); }
	bool isMember(const Permutation& perm) { return G.sifts(perm); }
	
	unsigned long burnside(unsigned int k) const;
	template<class PDomain, class Action>
		OrbitSet<Permutation, PDomain> orbit(const PDomain& item, Action action) const;

	GroupElementIterator elementsBegin() const;
	GroupElementIterator elementsEnd() const;
	
	bool operator==(const Group& rhs) const;
	bool operator!=(const Group& rhs) const { return !(*this == rhs); }
private:
	unsigned int v;
	std::list<Cycles> generators;
	
	// Built by the constructor
	std::list<Permutation::ptr> generatorPermutations;
	PermutationGroup G;
};

/**
 * Hash function overload for Group, as required by boost::hash
 */
std::size_t hash_value(const Group& G);

/**
 * This is a weak ordering for Group that will make Group work with containers such as std::map, which
 * requires a weak ordering of keys.  This should not be used as a general-purpose weak ordering, as
 * Groups are not meant to be ordered.
 *
 * This assumes that PermutationGroup's base (B) and strong generating set(S) uniquely describe the PermutationGroup.
 */
struct GroupWeakOrdering {
	bool operator()(const Group& lhs, const Group& rhs) const;
};

/**
 * GroupElementIterator is an input iterator for iterating through the elements of a group.
 * In the boost iterator hierarchy, it is a readable, swappable, single pass iterator.
 * (this iterator is not a forward traversal iterator as it cannot be default constructed, but
 * it can otherwise be used as one; iterator values are dereferenceable more than once)
 */
class GroupElementIterator : public std::iterator<std::input_iterator_tag, Permutation> {
	typedef std::vector<std::list<unsigned long>::const_iterator> TransversalIterators;
	friend class Group;
public:
	bool operator==(const GroupElementIterator& rhs) { return state == rhs.state; }
	bool operator!=(const GroupElementIterator& rhs) { return state != rhs.state; }
	GroupElementIterator& operator++();
	GroupElementIterator operator++(int);
	Permutation operator*() { return dereference(); }
	Permutation operator->() { return dereference(); }
private:
	GroupElementIterator(const boost::shared_ptr<Group>& G);
	
	Permutation dereference() const;
	void increment();
	
	boost::shared_ptr<Group> G;
	
	TransversalIterators begins;	// List of all begin() iterators for each transversal
	TransversalIterators ends;		// List of all end() iterators for each transversal
	TransversalIterators state;		// Iterator state
};

/* MEMBER TEMPLATE FUNCTIONS ***************************************************************** */

/**
 * Computes the orbit of item under the group.
 * 
 * @param <PDomain> Any type
 * @param <Action> A function or function object type, as defined by OrbitSet's <Action>.
 *                 Specifically, it must be a function (or function object with operator()) like
 *                 PDomain action(const Permutation& perm, PDomain item);
 * @param item The item for which the orbit is over.
 * @param action The group action.
 */
template<class PDomain, class Action>
OrbitSet<Permutation, PDomain> Group::orbit(const PDomain& item, Action action) const {
	OrbitSet<Permutation, PDomain> result;
	result.orbit(item, generatorPermutations, action);		// TODO - replace with SGS?
	return result;
}

#endif