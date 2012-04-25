#include <vector>
#ifndef SOLVER_H
#define SOLVER_H

/**
 * Solver is the base class for algorithms that find integer solutions to matrix equations.  Solvers need only to
 * differentiate between equations where solutions exist and where they do not; the number of solutions found does not
 * matter.
 *
 * There is no formal interface on how the equation is to be input, but Solver does prescribe the output format.
 */
class Solver {
public:
	typedef std::vector<int> SolutionVector;
	
	/**
	 * Solves the matrix equation.
	 * @return true if there is a solution, and false if there is no solution.
	 */
	virtual bool solve() = 0;
	
	/**
	 * Returns any solutions found by this solver.  The behaviour of this method is undefined if solve() has not been previously
	 * called.  If solve() returns false, then this method must return an empty vector.
	 */
	virtual const std::vector<SolutionVector>& getSolutionVectors() const = 0;
};

#endif