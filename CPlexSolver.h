#include <boost/multi_array.hpp>

#include <ilcplex/ilocplex.h>

#include "Solver.h"

#ifndef CPlexSolver_h
#define CPlexSolver_h

typedef boost::multi_array<int, 2> Matrix;

/**
 * Solver of the Kramer-Mesner Matrix Equation via CPlex.
 */
class CPlexSolver : public Solver {
public:
	CPlexSolver(const Matrix& A, unsigned int lambda);
	~CPlexSolver() { env.end(); }
	
	bool solve();
	const std::vector<SolutionVector>& getSolutionVectors() const { return solutionVectors; }
private:
	IloEnv env;
	IloModel model;
	IloBoolVarArray vars;
	IloRangeArray constraints;
	
	// Computed during solve()
	std::vector<SolutionVector> solutionVectors;
};

#endif
