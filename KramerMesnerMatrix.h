#include <boost/multi_array.hpp>

#include "Group.h"
#include "utils.h"

#ifndef KramerMesnerMatrix_h
#define KramerMesnerMatrix_h

typedef boost::multi_array<int, 2> Matrix;

/**
 * Convenience class that stores all the key data about a Kramer-Mesner matrix.
 */
class KramerMesnerMatrix {
public:
	KramerMesnerMatrix(const Group& G, const std::vector<Subset>& rowLabels, const std::vector<Subset>& columnLabels, const Matrix& matrix);
	
	const std::vector<Subset>& getRowLabels() const { return rowLabels; }
	const std::vector<Subset>& getColumnLabels() const { return columnLabels; }
	const Matrix& getMatrix() const { return matrix; }
	
	static KramerMesnerMatrix computeMatrix(const Group& G, unsigned int t, unsigned int k);
private:
	boost::shared_ptr<const Group> G;
	std::vector<Subset> rowLabels;
	std::vector<Subset> columnLabels;
	Matrix matrix;
};

#endif
