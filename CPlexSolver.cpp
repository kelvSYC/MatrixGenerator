#include "CPlexSolver.h"

CPlexSolver::CPlexSolver(const Matrix& A, unsigned int lambda) :
env(), model(env), vars(env), constraints(env),
solutionVectors() {
	// Add variables
	for (int i = 0; i < A.shape()[1]; i++) {
		vars.add(IloBoolVar(env));						// For simple t-designs
	}
	
	// Add constraints
	for (int i = 0; i < A.shape()[0]; i++) {
		constraints.add(IloRange(env, lambda, lambda));		// RHS = lambda in every equation
	}
	
	// Set up model to have as few block orbits as possible
	IloObjective objective = IloMinimize(env);
	for (int i = 0; i < A.shape()[1]; i++) {
		objective.setLinearCoef(vars[i], 1);
	}
	
	// Set up constraints according to input matrix
	for (int i = 0; i < A.shape()[0]; i++) {
		for (int j = 0; j < A.shape()[1]; j++) {
			constraints[i].setLinearCoef(vars[j], A[i][j]);
		}
	}
	
	// TODO - names for constraints/vars may be necessary
	// Might have to name these after the row/col labels for K-M Matrix
	
	// Finish setting up the model
	model.add(objective);
	model.add(constraints);
}

bool CPlexSolver::solve() {
	IloCplex cplex(model);
	cplex.setParam(IloCplex::MIPEmphasis, CPX_MIPEMPHASIS_FEASIBILITY);
	
	if (cplex.solve()) {
		IloNumArray vals(env);
		cplex.getValues(vals, vars);
		
		// Populate solutionVectors from vals
		SolutionVector solution(vals.getSize());
		for (int i = 0; i < vals.getSize(); i++) {			// IloNumArray does not have iterators, so we do it the hard way
			solution[i] = vals[i];
		}
		solutionVectors.push_back(solution);
		
		return true;
	} else {
		return false;
	}
}