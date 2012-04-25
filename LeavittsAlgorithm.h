#include <boost/multi_array.hpp>

#include "Solver.h"
#include "utils.h"

#ifndef LEAVITT_H
#define LEAVITT_H

typedef boost::multi_array<int, 2> Matrix;

/**
 * Implementation of Leavitt's Algorithm, which finds all 0-1 solutions of the integral matrix equation
 * AU = B, where A is a m x n matrix and B is an m x r matrix.  This solver is specifically tuned to an
 * input Kramer-Mesner Matrix.
 */
class LeavittSolver : public Solver {
public:
	LeavittSolver(const Matrix&A, unsigned int lambda);
	
	bool solve();
	// TODO - does not implement getSolutionVectors()
private:
	Matrix A;
	Matrix B;						// = lambda * [1 1 ...]
	Matrix U;						// = 0
	std::vector<unsigned int> F;	// = [0, .., n]
	
	// The following are the individual operation subroutines needed by solve().
	void gauss(unsigned int row);								// Divide a row in [A B] by the GCD of the elements therein
	void gauss(unsigned int row1, unsigned int row2);			// Swaps two rows in A and B
	void gauss(int alpha, unsigned int row1, unsigned row2);	// Adds alpha * row1 to row2 in [A B]
	void expand(int p, unsigned int row);						// Unconditional expand, using mod p
	void expand(unsigned int row);								// Conditional expand, using absolute sizes
	void contract1(unsigned int row);							// Remove columns in B/U not in the 0-1 span of row elements of A
	void contract2(unsigned int row);							// Remove columns in B/U where GCD of row elements of A does not divide corresponding entry in B
	void resolve(unsigned int row);								// Removes "identity row" in A
};

#endif