#include "KramerMesnerMatrix.h"
#include "KMCompute.h"

KramerMesnerMatrix::KramerMesnerMatrix(const Group& _G, const std::vector<Subset>& _rowLabels, const std::vector<Subset>& _columnLabels, const Matrix& _matrix) : G(_G.shared_from_this()), rowLabels(_rowLabels), columnLabels(_columnLabels), matrix(_matrix) {}

KramerMesnerMatrix KramerMesnerMatrix::computeMatrix(const Group& G, unsigned int t, unsigned int k) {
	return computeKMMatrix(G, t, k);
}