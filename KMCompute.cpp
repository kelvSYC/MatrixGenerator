#include "AnchorSet.h"
#include "Discriminator.h"
#include "GInvariant.h"
#include "KMBuilder.h"
#include "KMCompute.h"
#include "KMStrategy.h"

#include <boost/scoped_ptr.hpp>

typedef std::vector<GInvariant::ptr> GInvariantList;

/**
 * Convenience function for Matrix assignment.
 * boost::multi_array requires that matrices be of the same size before using operator=().
 */
inline void assignMatrix(Matrix& lhs, const Matrix& rhs) {
	lhs.resize(boost::extents[rhs.shape()[0]][rhs.shape()[1]]);
	lhs = rhs;
}

/**
 * Divides a matrix by a scalar in place.
 */
void scalarDivide(int c, Matrix& A) {
	for (int i = 0; i < A.shape()[0]; i++) {
		for (int j = 0; j < A.shape()[1]; j++) {
			A[i][j] /= c;
		}
	}
}

/**
 * Creates a new matrix by multiplying two existing matrices (in the naive manner).
 *
 * Precondition: the matrices must be multiplication-compatible.
 */
Matrix matrixMultiply(const Matrix& A, const Matrix& B) {
	Matrix result(boost::extents[A.shape()[0]][B.shape()[1]]);
	
	for (int i = 0; i < result.shape()[0]; i++) {
		for (int j = 0; j < result.shape()[1]; j++) {
			for (int k = 0; k < A.shape()[1]; k++) {
				result[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	
	return result;
}

/**
 * Compute the Kramer-Mesner matrix.
 *
 * Precondition: t < k
 */
KramerMesnerMatrix computeKMMatrix(const Group& G, unsigned int t, unsigned int k) {
	std::vector<KMBuilderOutput> builderOutputs;			// stores the relevant data for k = 2 onwards
	Matrix A;
	
	for (int i = 2; i <= k; i++) {
		boost::scoped_ptr<KMBuilder> builder;
		
		// Get orbit representatives of (i - 1)-subsets
		std::vector<Subset> orbitReps;
		if (i == 2) {
			// As it is the first time, we have to compute the orbit representatives of singleton subsets...
			Subset pointsRemaining = generateX(G.getNumPoints());
			while (!pointsRemaining.empty()) {
				typedef OrbitSet<Permutation, unsigned long>::const_iterator OrbitSetIterator;
				
				unsigned long point = *(pointsRemaining.begin());	// A new representative...
				Subset singletonSet = makeSingletonSet(point);		// ...as a set
				orbitReps.push_back(singletonSet);
				
				// Compute the orbit of the point, and remove it from the remaining points
				OrbitSet<Permutation, unsigned long> orbit = G.orbit(point, Transversal::TrivialAction());
				for (OrbitSetIterator it = orbit.begin(); it != orbit.end(); it++) {
					pointsRemaining.erase(pointsRemaining.find(*it));
				}
			}
			
			builder.reset(new KMBuilder(G, i, orbitReps));
		} else {
			// We get them from what we computed earlier
			KMBuilderOutput& input = builderOutputs[i - 3];
			orbitReps = input.getNewReps();
			
			builder.reset(new KMBuilder(G, i, orbitReps, input.getPrunerData()));
		}
		
		KMBuilderOutput builderOutput = builder->build();
		builderOutputs.push_back(builderOutput);
		
		// From the identity that A[t][k] = A[t][s] * A[s][k] / combinat(k - t, k - s) for any s
		// between t and k, we get the following:
		if (i == t + 1) {
			assignMatrix(A, builderOutput.getNewMatrix());
		} else if (i > t + 1) {
			assignMatrix(A, matrixMultiply(A, builderOutput.getNewMatrix()));
			scalarDivide(i - t, A);
		}
	}
	
	return KramerMesnerMatrix(G, builderOutputs[t - 2].getNewReps(), builderOutputs[k - 2].getNewReps(), A);
}