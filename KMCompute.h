#include <boost/multi_array.hpp>

#ifndef KMCOMPUTE_H
#define KMCOMPUTE_H

#include "Group.h"
#include "KramerMesnerMatrix.h"

typedef boost::multi_array<int, 2> Matrix;

KramerMesnerMatrix computeKMMatrix(const Group& G, unsigned int t, unsigned int k);

#endif