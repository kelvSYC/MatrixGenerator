#include "TrivialDiscriminator.h"

bool TrivialDiscriminatorWeakOrdering::operator()(const TrivialDiscriminator& lhs, const TrivialDiscriminator& rhs) const {
	return GroupWeakOrdering()(lhs.getGroup(), rhs.getGroup());
}

/**
 * Compares two TrivialDiscriminators for equality.
 *
 * Two TrivialDiscriminators are equal if and only if they are over the same group; TrivialDiscriminators do not have any
 * data beyond that required by GInvariant.
 */
bool TrivialDiscriminator::equals(const GInvariant& rhs) const {
	if (!GInvariant::equals(rhs)) return false;
	return true;
}

std::size_t TrivialDiscriminator::hash() const {
	std::size_t seed = 0;
	boost::hash_combine(seed, getGroup());
	return seed;
}