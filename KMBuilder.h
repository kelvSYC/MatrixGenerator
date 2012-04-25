#include <boost/any.hpp>
#include <boost/multi_array.hpp>
#include <boost/optional.hpp>
#include <boost/timer/timer.hpp>

#include "GInvariant.h"
#include "Pruner.h"

#ifndef KMBUILDER_H
#define KMBUILDER_H

class KMBuilderOutput;

typedef boost::multi_array<int, 2> Matrix;

/**
 * Builder class for the Kramer-Mesner matrix.  The builder itself requiress a Group and a list of
 * orbit representatives over (k-1)-subsets; the building process requires a list of G-invariant
 * functions over (k-1)-subsets.
 * 
 * Once isReady() returns true, the items produced (the list of orbit representatives for k-subsets,
 * a discriminator over k-subsets, and the Kramer-Mesner matrix A[k-1][k]) are returned by querying
 * the various functions; at this point no further modifications are made to the instance.
 */
class KMBuilder {
public:
	KMBuilder(const Group& G, unsigned int k, const std::vector<Subset>& orbitReps, const boost::any& prunerData = boost::any());
	
	KMBuilderOutput build();
private:
	boost::shared_ptr<const Group> G;
	std::vector<Subset> orbitReps;		// Labels for each row of A
	
	// Built by the constructor
	unsigned long rho;					// Number of orbits of k-subsets
	
	// Depending on whether rho == 1, we may need a pruner
	boost::shared_ptr<Pruner> pruner;
	
	// Time each iteration through this
	boost::timer::auto_cpu_timer timer;
};

/**
 * Class representing the output of a KMBuilder.  This includes the orbit representatives for k-subsets,
 * the Kramer-Mesner matrix, and any supplementary data produced by the pruner.
 *
 * As the name implies, new instances (that weren't created from copy construction) may only be created by a KMBuilder.
 */
class KMBuilderOutput {
	friend class KMBuilder;				// Only KMBuilder may build it
	
	KMBuilderOutput(const std::vector<Subset>& newReps_, const Matrix& A_, const boost::any& _prunerData = boost::any()) : newReps(newReps_), A(A_), prunerData(_prunerData) {}
public:
	// Explicit assignment operator - needed due to the assignment semantics of A
	KMBuilderOutput& operator=(const KMBuilderOutput& other) {
		if (this == &other) return *this;									// Prevent self-assign
		newReps = other.newReps;
		A.resize(boost::extents[other.A.shape()[0]][other.A.shape()[1]]);	// Resize A before assign
		A = other.A;
		return *this;
	}
	
	std::vector<Subset> getNewReps() const { return newReps; }
	Matrix getNewMatrix() const { return A; }
	boost::any getPrunerData() const { return prunerData; }
private:
	std::vector<Subset> newReps;
	Matrix A;
	boost::any prunerData;
};
#endif