#include <algorithm>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/math/common_factor_rt.hpp>

#include "LeavittsAlgorithm.h"

void deleteRow(Matrix& A, size_t row) {
	for (int i = row; i < A.shape()[0] - 1; i++) {
		for (int j = 0; j < A.shape()[1]; j++) {
			A[i][j] = A[i+1][j];
		}
	}
	A.resize(boost::extents[A.shape()[0] - 1][A.shape()[1]]);
}

void deleteColumn(Matrix& A, size_t column) {
	for (int i = 0; i < A.shape()[0]; i++) {
		for (int j = column; j < A.shape()[1] - 1; j++) {
			A[i][column] = A[i][column + 1];
		}
	}
	A.resize(boost::extents[A.shape()[0]][A.shape()[1] - 1]);
}

/**
 * Augments a matrix on the right with another matrix.  The two matrices must have the same number of rows.
 */
void augmentRight(Matrix& A, Matrix& B) {
	size_t border = A.shape()[1];
	A.resize(boost::extents[A.shape()[0]][A.shape()[1] + B.shape()[1]]);
	for (int i = 0; i < A.shape()[0]; i++) {
		for (int j = 0; j < B.shape()[1]; j++) {
			A[i][border + j] = B[i][j];
		}
	}
}

/**
 * Takes the GCD of a list of integral types.
 *
 * @param <IntegerType> The integral type the GCD is to be taken over.  Corresponds to the <IntegralType> param in boost::math::gcd_evaluator.
 * @param <InputIterator> An input iterator class, as defined in the STL.
 */
template <class IntegerType, class InputIterator>
IntegerType gcd(InputIterator first, InputIterator last) {
	return std::accumulate(first, last, 0, boost::math::gcd_evaluator<IntegerType>());
}

/**
 * Takes the 0-1 span of a sequence of integers.
 *
 * @param <InputIterator> An input iterator class, iterating through a sequence of integers.
 */
template <class InputIterator>
Subset bspan(InputIterator first, InputIterator last) {
	Subset result;
	result.insert(0);	// All spans start with 0
	for (InputIterator it = first; it != last; it++) {
		Subset newMembers;
		
		// add *it to each member of newMembers
		for (Subset::const_iterator jt = result.begin(); jt != result.end(); jt++) {
			newMembers.insert(*jt + *it);
		}
		
		// add copy to original
		result.insert(newMembers.begin(), newMembers.end());
	}
	return result;
}

LeavittSolver::LeavittSolver(const Matrix& _A, unsigned int lambda) :
A(_A), B(boost::extents[A.shape()[0]][1]), U(boost::extents[A.shape()[1]][1]),
F(boost::counting_iterator<unsigned int>(0), boost::counting_iterator<unsigned int>(A.shape()[1])) {
	// Initialize the RHS matrix
	for (int i = 0; i < B.shape()[0]; i++) {
		B[i][0] = lambda;
	}
}

/**
 * Gauss operation G[i]
 * 
 * Divides row i in [A B] by the GCD of the elements therein.
 */
void LeavittSolver::gauss(unsigned int row) {
	int divisor = boost::math::gcd(gcd<int>(A[row].begin(), A[row].end()), gcd<int>(B[row].begin(), B[row].end()));
	
	for (int j = 0; j < A.shape()[1]; j++) A[row][j] /= divisor;
	for (int j = 0; j < B.shape()[1]; j++) B[row][j] /= divisor;
}

/**
 * Gauss operation G[i,j]
 *
 * Swaps row i and row j of [A B]
 */
void LeavittSolver::gauss(unsigned int row1, unsigned int row2) {
	// For some reason std::swap(A[row1], A[row2]) doesn't work, so we'll do it the hard way...
	for (int j = 0; j < A.shape()[1]; j++) {
		std::swap(A[row1][j], A[row2][j]);
	}
	for (int j = 0; j < B.shape()[1]; j++) {
		std::swap(B[row1][j], B[row2][j]);
	}
}

/**
 * Gauss operation G[alpha;i,j]
 *
 * Adds alpha * row i to row j of [A B]
 */
void LeavittSolver::gauss(int alpha, unsigned int row1, unsigned int row2) {
	for (int i = 0; i < A.shape()[1]; i++) {
		A[row2][i] += alpha * A[row1][i];
	}
	for (int i = 0; i < B.shape()[1]; i++) {
		B[row2][i] += alpha * B[row1][i];
	}
}

/**
 * Expansion Operation E[p;i]
 *
 * Add a new row to A, with contents of row i mod p.  Do the same with B.
 *
 * Then, for each column j in B, and for each s in the 0-1 span of the new row
 * in A, if s is congruent to B[i][j] mod p, then add a new column of B as a
 * copy of column j in B (except in the new row, where s is the corresponding
 * entry), and add a new column of U as a copy of column j in U.
 */
void LeavittSolver::expand(int p, unsigned int row) {
	if (p <= 0) return;
	
	// Add new column to A
	A.resize(boost::extents[A.shape()[0] + 1][A.shape()[1]]);
	for (int j = 0; j < A.shape()[1]; j++) {
		A[A.shape()[0] - 1][j] = A[row][j] % p;
	}
	
	// Add new column to B
	B.resize(boost::extents[B.shape()[0] + 1][B.shape()[1]]);
	for (int j = 0; j < B.shape()[1]; j++) {
		B[B.shape()[0] - 1][j] = B[row][j] % p;
	}
	
	Subset span = bspan(A[A.shape()[0] - 1].begin(), A[A.shape()[0] - 1].end());
	size_t numOrig = B.shape()[1];
	for (int j = 0; j < numOrig; j++) {
		for (Subset::const_iterator it = span.begin(); it != span.end(); it++) {
			if (*it % p == B[row][j] % p) {
				// Add new column to B
				B.resize(boost::extents[B.shape()[0]][B.shape()[1] + 1]);
				for (int k = 0; k < B.shape()[0] - 1; k++) {
					B[k][B.shape()[1] - 1] = B[k][j];
				}
				B[B.shape()[0] - 1][B.shape()[1] - 1] = *it;
				
				// Add new column to U
				U.resize(boost::extents[U.shape()[0]][U.shape()[1] + 1]);
				for (int k = 0; k < U.shape()[0]; k++) {
					U[k][U.shape()[1] - 1] = U[k][j];
				}
			}
		}
	}
}

/**
 * Expansion Operation E[i]
 *
 * If all of the elements of row i in A are either -1, 0, or 1, then add a row
 * in A with entry 1 if the corresponding entry in row i is -1, and 0 otherwise.
 * Also, add a row in B with entries the same as row i of B.
 *
 * Then, for each column j in B, if for each s in the 0-1 span of the new row in
 * A larger than -B[i][j], add a new column of B as a copy of column j of B
 * (except in the new row, where the corresponding entry is s), and add a new
 * column of U as a copy of column j of U.
 */
void LeavittSolver::expand(unsigned int row) {
	for (int j = 0; j < A.shape()[1]; j++) {
		if (A[row][j] != 0 && A[row][j] != 1 && A[row][j] != -1) return;
	}
	
	// Add the new row in A
	A.resize(boost::extents[A.shape()[0] + 1][A.shape()[1]]);
	for (int j = 0; j < A.shape()[1]; j++) {
		A[A.shape()[0] - 1][j] = (A[row][j] == -1) ? 1 : 0;
	}
	
	// Add the new row in B
	B.resize(boost::extents[B.shape()[0] + 1][B.shape()[1]]);
	for (int j = 0; j < B.shape()[1]; j++) {
		B[B.shape()[0] - 1][j] = B[row][j];
	}
	
	Subset span = bspan(A[A.shape()[0] - 1].begin(), A[A.shape()[0] - 1].end());
	size_t numOrig = B.shape()[1];
	for (int j = 0; j < numOrig; j++) {
		for (Subset::const_iterator it = span.begin(); it != span.end(); it++) {
			if (*it > -B[row][j]) {
				// Add new column to B
				B.resize(boost::extents[B.shape()[0]][B.shape()[1] + 1]);
				for (int k = 0; k < B.shape()[0] - 1; k++) {
					B[k][B.shape()[1] - 1] = B[k][j];
				}
				B[B.shape()[0] - 1][B.shape()[1] - 1] = *it;
				
				// Add new column to U
				U.resize(boost::extents[U.shape()[0]][U.shape()[1] + 1]);
				for (int k = 0; k < U.shape()[0]; k++) {
					U[k][U.shape()[1] - 1] = U[k][j];
				}
			}
		}
	}
}

/**
 * Contraction Operation C1[i]
 *
 * If B[i][j] is not in the 0-1 span of {elements of row i of A}, then remove column j from B and U.
 */
void LeavittSolver::contract1(unsigned int row) {
	for (int j = 0; j < A.shape()[1]; j++) {
		if (bspan(A[row].begin(), A[row].end()).count(B[row][j]) == 0) {
			deleteColumn(B, j);
			deleteColumn(U, j);
			j--;	// Keep index in check
		}
	}
}

/**
 * Contraction Operation C2[i]
 *
 * If the GCD of the elements of row i of A does not divide B[i][j], then delete column j of B and U.
 */
void LeavittSolver::contract2(unsigned int row) {
	unsigned int rowgcd = gcd<int>(A[row].begin(), A[row].end());
	
	for (int j = 0; j < B.shape()[1]; j++) {
		if (B[row][j] % rowgcd) {
			deleteColumn(B, j);
			deleteColumn(U, j);
			j--;	// Keep index in check
		}
	}
}

/**
 * Resolution Operation R[i]
 * 
 * If row i of A has all zeroes except at col j, then we substitute row i of B into row F[j] of U,
 * delete row i of both A and B, delete column j of A, and delete F[j].
 */
void LeavittSolver::resolve(unsigned int row) {
	int col = -1;
	for (int j = 0; j < A.shape()[1]; j++) {
		if (A[row][j] == 0) continue;			// Column with entry that's 0
		else if (A[row][j] != 1) return;		// Column with entry that's not 1
		else {
			if (col != -1) return;				// Existing row with entry 1
			col = j;
		}
	}
	if (col == -1) return;
	
	// Now we have exactly one column with 1, and 0s everywhere else
	U[F[col]] = B[row];	// substitute row i of B into row F[col] of U
	
	deleteRow(A, row);
	deleteRow(B, row);
	deleteColumn(A, col);
	F.erase(F.begin() + col);
}

bool LeavittSolver::solve() {
	for (int i = 0; i < A.shape()[1] - 1; i++) {		// Step 5, 6, 24, 25
		for (int h = i + 1; h < A.shape()[1]; h++) {	// Step 7, 8, 22, 23
			bool boole = true;							// Step 9
			for (int j = 0; j < A.shape()[0]; j++) {	// Step 10, 12
				boole &= (A[j][i] == A[j][h]);			// Step 11 (The paper had the indices backwards for some reason...)
			}
			if (boole) {								// Step 13
				Matrix U1(U);							// Step 14
				Matrix B1(B);
				
				// From the paper: this should be a loop over all columns of B1/U1 (step 15,19), but we start with only one col
				U1[h][0] = 1;							// Step 16
				for (int k = 0; k < A.shape()[0]; k++) {	// Step 17
					B1[k][0] -= A[k][h];				// Step 18
				}
				// From the paper: the loop over the cols of B1/U1 ends here
				
				augmentRight(B, B1);					// Step 20 (At least that's what I think "Set B = [B;B1]" meant...)
				augmentRight(U, U1);
				
				F.erase(F.begin() + h);					// Step 21: Delete F[h] and column h of A
				deleteColumn(A, h);
			}
		}
	}
	
	while (A.shape()[1] > 0) {							// Step 54
		int s = -1;										// Step 26: Let s be the index of the first row in A that's not a 0-1 vector
		for (int i = 0; i < A.shape()[0]; i++) {		// ... so does that mean row s is the first row with an entry that's not
			bool nbRow = false;							// 0 or 1?
			for (int j = 0; j < A.shape()[1]; j++) {
				if (A[i][j] != 0 && A[i][j] != 1) {
					nbRow = true;
					break;
				}
			}
			if (nbRow) {
				s = i;
				break;
			}
		}
		if (s == -1) s = A.shape()[0];					// So A is a 0-1 matrix if s == -1 at this point.
		
		int index = 0;									// Step 27: (27-34) Find the index of the row with the fewest odd entries
		int numodd = A.shape()[1];
		for (int i = s; i < A.shape()[0]; i++) {		// Step 28, 33, 34
			contract2(i);								// Step 29
			gauss(i);
			contract1(i);
			
			int q = 0;									// Step 30: Let q be the number of odd entries in ith row of A
			for (size_t j = 0; j < A.shape()[1]; j++) {
				if ((A[i][j] & 1) != 0) q++;
			}
			
			if (q < numodd) {							// Step 31
				index = i;								// Step 32
				numodd = q;
			}
		}
		
		expand(2, index);								// Step 35
		gauss(-1, A.shape()[0] - 1, index);				// (says "E[-1;m,index]" in the paper, but this is probably what it meant)
		contract2(index);
		gauss(index);
		gauss(A.shape()[0] - 1, s);						// A[s] is now the new row from expand()
		
		for (int i = 0; i < A.shape()[0]; i++) {		// Step 36, 37, 41, 42
			if (i != s) {								// Step 38
				gauss(-A[i][j], s, i);					// Step 39 (Says A[i][j] without what j is...)
			}
			contract1(i);								// Step 40
		}
		
		for (int i = A.shape()[0]; i < s;) {			// Step 43, 44, 53 (Really confusing here...)
			int count = 0;								// Step 45
			for (int j = 0; j < A.shape()[1]; j++) {	// Step 46, 50
				if (A[i][j] != 0) count++;				// Step 47
				if (A[i][j] != -1) continue;			// Step 48
				expand(i);								// Step 49
				gauss(1, A.shape()[0] - 1, i);
			}
			if (count != 1) i++;						// Step 51
			else s--;
			
			resolve(i);									// Step 52
		}
	}
	
	if (B.shape()[1] == 0) {							// Step 55
		// No solution
		return false;
	} else {
		// Each column in U is a solution
		return true;
	}
}