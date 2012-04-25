#include <iostream>
#include <list>

#include <boost/array.hpp>
#include <boost/make_shared.hpp>

#include "Group.h"
#include "Groups.h"
#include "KMCompute.h"
#include "CPlexSolver.h"

typedef std::vector<int> SolutionVector;

template<class T>
struct NullDeleter {
	void operator()(T* t) const {}
};

KramerMesnerMatrix createKMMatrix() {
	Group G = createProjectiveSemilinear232();
//	Group G = createProjectiveSpecialLinear35();
	boost::shared_ptr<Group> gPtr(&G, NullDeleter<Group>());	// Owning instance so the KM routines don't barf
//	return KramerMesnerMatrix::computeMatrix(*gPtr, 8, 10);
	return KramerMesnerMatrix::computeMatrix(*gPtr, 6, 8);		// But now there's a dangling reference in the return!  What to do...
}

Matrix genSampleMatrix() {
	// Sample Matrix from computeKMMatrix(G, 6, 8)
	boost::array<unsigned long, 1261> entries = {
		5, 4, 4, 6, 6, 6, 14, 8, 6, 6, 6, 6, 4, 6, 4, 4, 6, 4, 6, 2, 2, 4, 8, 2, 6, 4, 6, 6, 2, 4, 8, 4, 2, 2, 3, 4, 8, 2, 6, 4, 2, 4, 6, 4, 4, 1, 6, 6, 4, 4, 6, 4, 2, 4, 2, 6, 4, 2, 1, 2, 2, 6, 4, 3, 2, 2, 4, 6, 2, 1, 2, 6, 3, 4, 6, 4, 1, 2, 4, 4, 2, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		6, 8, 4, 6, 6, 8, 6, 8, 8, 6, 4, 4, 8, 8, 6, 4, 4, 8, 6, 10, 8, 4, 6, 6, 6, 8, 4, 4, 4, 8, 6, 2, 2, 4, 0, 2, 2, 2, 0, 4, 2, 0, 4, 2, 4, 1, 2, 2, 2, 2, 0, 4, 0, 4, 2, 2, 2, 2, 3, 2, 1, 4, 2, 0, 6, 2, 2, 4, 4, 1, 2, 0, 0, 6, 2, 4, 3, 6, 4, 5, 4, 1, 4, 4, 4, 4, 2, 4, 2, 1, 4, 1, 4, 2, 0, 0, 0, 
		4, 6, 6, 5, 5, 6, 6, 3, 2, 4, 4, 6, 5, 7, 5, 2, 7, 6, 6, 2, 4, 7, 4, 5, 5, 3, 3, 5, 3, 3, 4, 5, 4, 4, 3, 4, 4, 5, 8, 5, 2, 2, 3, 5, 4, 4, 5, 3, 0, 4, 1, 4, 4, 5, 0, 2, 3, 2, 2, 1, 2, 2, 2, 2, 7, 5, 2, 3, 4, 2, 5, 3, 0, 3, 4, 2, 1, 1, 5, 0, 4, 0, 5, 6, 6, 3, 4, 4, 3, 1, 2, 2, 1, 4, 7, 3, 0, 
		3, 6, 4, 3, 5, 3, 0, 3, 3, 2, 0, 3, 4, 7, 5, 6, 4, 4, 6, 5, 3, 4, 5, 4, 4, 4, 6, 4, 2, 4, 4, 6, 6, 7, 2, 4, 5, 4, 3, 6, 2, 3, 5, 6, 4, 2, 3, 5, 6, 3, 6, 4, 6, 3, 3, 4, 4, 5, 2, 2, 3, 4, 3, 2, 3, 2, 5, 2, 4, 1, 4, 5, 4, 3, 5, 4, 2, 5, 3, 0, 2, 0, 1, 4, 1, 5, 5, 5, 2, 2, 5, 1, 3, 4, 4, 1, 1, 
		0, 0, 2, 2, 4, 2, 0, 8, 8, 2, 0, 4, 4, 0, 0, 2, 2, 2, 4, 6, 12, 2, 2, 6, 8, 4, 4, 4, 0, 4, 6, 4, 4, 2, 0, 0, 4, 4, 4, 0, 2, 3, 2, 4, 4, 2, 6, 4, 4, 2, 6, 2, 6, 2, 3, 8, 2, 4, 0, 1, 3, 6, 6, 1, 6, 4, 8, 8, 10, 2, 6, 8, 7, 4, 6, 4, 6, 8, 4, 5, 2, 1, 4, 0, 4, 2, 2, 0, 4, 0, 10, 0, 1, 8, 6, 2, 0, 
		0, 3, 2, 1, 2, 4, 8, 6, 3, 2, 2, 5, 4, 4, 2, 3, 2, 6, 1, 4, 5, 3, 7, 5, 5, 5, 8, 2, 2, 5, 4, 3, 3, 4, 2, 5, 2, 2, 6, 4, 1, 0, 8, 3, 5, 1, 4, 5, 4, 3, 1, 3, 6, 3, 1, 7, 6, 3, 2, 2, 2, 7, 2, 2, 3, 3, 3, 5, 6, 1, 4, 7, 2, 5, 5, 6, 4, 7, 5, 5, 2, 0, 4, 4, 5, 5, 4, 4, 2, 1, 6, 1, 3, 6, 2, 3, 1, 
		2, 4, 4, 4, 4, 4, 6, 2, 6, 12, 4, 6, 4, 2, 4, 6, 6, 2, 4, 4, 2, 4, 2, 4, 2, 2, 2, 6, 2, 2, 2, 2, 6, 6, 4, 4, 4, 12, 4, 6, 3, 3, 4, 4, 4, 3, 6, 2, 2, 10, 0, 6, 2, 10, 2, 0, 4, 4, 0, 2, 0, 2, 4, 4, 4, 2, 4, 2, 0, 0, 4, 2, 0, 4, 2, 4, 1, 2, 4, 0, 8, 1, 2, 6, 2, 4, 8, 6, 3, 9, 2, 3, 3, 2, 6, 0, 2, 
		0, 0, 6, 6, 4, 4, 0, 2, 4, 2, 0, 2, 2, 0, 4, 6, 2, 2, 2, 6, 4, 2, 2, 2, 0, 4, 4, 6, 2, 2, 2, 4, 4, 4, 0, 4, 4, 2, 2, 4, 3, 2, 2, 2, 2, 2, 4, 6, 10, 4, 6, 6, 4, 4, 3, 4, 4, 14, 3, 1, 4, 2, 4, 3, 2, 10, 6, 6, 4, 9, 6, 6, 1, 4, 4, 8, 2, 2, 2, 0, 10, 1, 4, 2, 6, 4, 6, 6, 3, 2, 2, 2, 2, 8, 4, 4, 2, 
		0, 0, 4, 6, 4, 4, 0, 2, 6, 4, 0, 2, 0, 0, 6, 4, 4, 2, 2, 2, 2, 4, 0, 6, 2, 6, 2, 4, 2, 2, 2, 4, 4, 2, 0, 4, 4, 2, 2, 2, 4, 3, 4, 2, 4, 1, 2, 4, 4, 6, 12, 6, 6, 2, 3, 4, 4, 4, 3, 3, 2, 6, 10, 3, 4, 4, 6, 4, 4, 3, 4, 4, 3, 2, 6, 4, 1, 2, 4, 0, 4, 1, 10, 4, 4, 12, 8, 6, 0, 1, 8, 9, 3, 6, 4, 4, 2, 
		3, 12, 6, 6, 0, 0, 0, 6, 6, 6, 0, 0, 0, 6, 0, 6, 12, 0, 0, 6, 0, 6, 0, 0, 0, 6, 0, 6, 0, 6, 0, 6, 6, 6, 9, 12, 12, 6, 6, 6, 6, 3, 6, 6, 12, 6, 6, 0, 0, 6, 0, 0, 0, 0, 3, 0, 6, 0, 6, 3, 0, 0, 6, 0, 0, 12, 6, 6, 0, 0, 0, 0, 0, 12, 0, 6, 0, 6, 6, 0, 6, 3, 6, 6, 6, 0, 0, 6, 0, 3, 0, 3, 3, 0, 6, 6, 0, 
		0, 5, 5, 5, 0, 5, 5, 0, 0, 5, 5, 0, 0, 0, 10, 5, 0, 5, 5, 0, 5, 5, 0, 5, 0, 5, 0, 0, 5, 5, 0, 5, 5, 0, 5, 10, 5, 10, 0, 5, 0, 0, 0, 10, 5, 0, 0, 5, 10, 5, 10, 5, 5, 10, 5, 5, 5, 5, 5, 5, 0, 0, 5, 0, 5, 5, 5, 0, 5, 5, 0, 0, 5, 5, 5, 0, 0, 5, 10, 0, 10, 0, 5, 10, 10, 0, 0, 0, 0, 5, 10, 0, 0, 5, 5, 0, 1, 
		0, 0, 10, 5, 5, 0, 0, 0, 0, 5, 0, 5, 10, 0, 5, 5, 5, 0, 10, 5, 0, 5, 5, 0, 5, 0, 0, 0, 0, 10, 5, 5, 0, 5, 0, 5, 5, 5, 5, 5, 0, 0, 0, 5, 0, 5, 5, 5, 10, 10, 10, 5, 5, 10, 5, 5, 10, 5, 5, 5, 0, 5, 5, 0, 5, 5, 0, 5, 0, 0, 5, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 0, 10, 0, 5, 5, 0, 10, 5, 5, 0, 5, 0, 0, 10, 5, 1, 
		0, 10, 0, 5, 5, 0, 0, 0, 0, 5, 0, 5, 10, 5, 5, 5, 0, 5, 0, 0, 0, 5, 5, 0, 5, 0, 0, 5, 0, 5, 10, 5, 10, 5, 5, 5, 0, 5, 0, 0, 0, 5, 0, 5, 5, 0, 5, 5, 5, 0, 5, 5, 0, 0, 0, 5, 5, 5, 0, 5, 5, 5, 15, 5, 5, 10, 5, 5, 5, 5, 10, 0, 0, 5, 0, 0, 0, 5, 5, 5, 10, 0, 5, 10, 5, 5, 10, 5, 0, 5, 0, 5, 0, 0, 5, 0, 1
	};
	
	Matrix A(boost::extents[13][97]);
	A.assign(entries.begin(), entries.end());
	
	return A;
}

int main (int argc, char * const argv[]) {
	std::cout << "Starting" << std::endl;
	KramerMesnerMatrix A = createKMMatrix();
	std::cout << "Done" << std::endl;
//	for (int i = 0; i < A.getMatrix().shape()[0]; i++) {
//		std::cout << "[";
//		for (int j = 0; j < A.getMatrix().shape()[1]; j++) {
//			std::cout << A.getMatrix()[i][j] << " ";
//		}
//		std::cout << "]" << std::endl;
//	}
	std::cout << (A.getMatrix() == genSampleMatrix()) << std::endl;
	// Test the solver now...
//	Matrix A = genSampleMatrix();
/*	CPlexSolver solver(A.getMatrix(), 36);
	if (solver.solve()) {
		std::cout << "Solution found" << std::endl;
		std::vector<SolutionVector> solutions = solver.getSolutionVectors();
		for (std::vector<SolutionVector>::const_iterator it = solutions.begin(); it != solutions.end(); it++) {
			std::cout << "Solution:" << std::endl;
			std::cout << "\t[";
			for (SolutionVector::const_iterator jt = it->begin(); jt != it->end(); jt++) {
				if (jt != it->begin()) std::cout << ", ";
				std::cout << *jt;
			}
			std::cout << "]" << std::endl;
			
			std::cout << "t-design:" << std::endl;
			for (SolutionVector::const_iterator jt = it->begin(); jt != it->end(); jt++) {
				for (int k = 0; k < *jt; k++) {
					Subset s = A.getColumnLabels()[jt - it->begin()];
					std::cout << "\tOrbit of: {";
					for (Subset::const_iterator zt = s.begin(); zt != s.end(); zt++) {
						if (zt != s.begin()) std::cout << ", ";
						std::cout << *zt;
					}
					std::cout << "}" << std::endl;
				}
			}
		}
	} else {
		std::cout << "No solution found" << std::endl;
	}
*/	
	return 0;
}
