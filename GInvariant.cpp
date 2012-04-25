#include <boost/functional/hash.hpp>

#include "GInvariant.h"

/**
 * Minimal implementation of an equality comparator.
 *
 * In theory, two GInvariants should be equal if for every set B (of suitable size) evaluate(B) ==
 * rhs.evaluate(B).  Since we can't tell a priori whether this is the case, we stick with an easier but
 * more stringent definition.
 * 
 * Two GInvariants are possibly equal if they are over the same group, but definitely different over
 * different groups.  Note that operator==() itself isn't defined over GInvariant to avoid comparing two
 * instances of different GInvariant subclasses; however, subclasses are required to have one, which must
 * call this method.
 */
bool GInvariant::equals(const GInvariant& rhs) const {
	if (typeid(*this) != typeid(rhs)) return false;
	return G == rhs.G;
}

std::size_t hash_value(const GInvariantEvaluationTask& task) {
	std::size_t hash = 0;
	boost::hash_combine(hash, task.getFn()->hash());
	boost::hash_combine(hash, task.getInput());
	return hash;
}