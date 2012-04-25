#include "Groups.h"

template<size_t N>
Cycle toCycle(const boost::array<unsigned long, N>& cycleAsArray) {
	return Cycle(cycleAsArray.begin(), cycleAsArray.end());
}

Group createProjectiveSemilinear232() {
	// Build our group - This is so much easier with C++0x and std::initializer_list...
	// (1 2 4 8 16)(3 6 12 24 17)(5 10 20 9 18)(7 14 28 25 19)(11 22 13 26 21)(15 30 29 27 23)(31)(32)(33)
	boost::array<unsigned long, 5> a1 = {1, 2, 4, 8, 16};
	boost::array<unsigned long, 5> a2 = {3, 6, 12, 24, 17};
	boost::array<unsigned long, 5> a3 = {5, 10, 20, 9, 18};
	boost::array<unsigned long, 5> a4 = {7, 14, 28, 25, 19};
	boost::array<unsigned long, 5> a5 = {11, 22, 13, 26, 21};
	boost::array<unsigned long, 5> a6 = {15, 30, 29, 27, 23};
	boost::array<unsigned long, 1> a7 = {31};
	boost::array<unsigned long, 1> a8 = {32};
	boost::array<unsigned long, 1> a9 = {33};
	Cycles a;
	a.push_back(toCycle(a1));
	a.push_back(toCycle(a2));
	a.push_back(toCycle(a3));
	a.push_back(toCycle(a4));
	a.push_back(toCycle(a5));
	a.push_back(toCycle(a6));
	a.push_back(toCycle(a7));
	a.push_back(toCycle(a8));
	a.push_back(toCycle(a9));
	
	// (1 18 30)(2 21 12)(3 10 28)(4 31 32)(5 24 14)(6 7 17)(8 25 27)(9 19 20)(11 15 13)(16 23 29)(22 33 26)
	boost::array<unsigned long, 3> b1 = {1, 18, 30};
	boost::array<unsigned long, 3> b2 = {2, 21, 12};
	boost::array<unsigned long, 3> b3 = {3, 10, 28};
	boost::array<unsigned long, 3> b4 = {4, 31, 32};
	boost::array<unsigned long, 3> b5 = {5, 24, 14};
	boost::array<unsigned long, 3> b6 = {6, 7, 17};
	boost::array<unsigned long, 3> b7 = {8, 25, 27};
	boost::array<unsigned long, 3> b8 = {9, 19, 20};
	boost::array<unsigned long, 3> b9 = {11, 15, 13};
	boost::array<unsigned long, 3> b10 = {16, 23, 29};
	boost::array<unsigned long, 3> b11 = {22, 33, 26};
	Cycles b;
	b.push_back(toCycle(b1));
	b.push_back(toCycle(b2));
	b.push_back(toCycle(b3));
	b.push_back(toCycle(b4));
	b.push_back(toCycle(b5));
	b.push_back(toCycle(b6));
	b.push_back(toCycle(b7));
	b.push_back(toCycle(b8));
	b.push_back(toCycle(b9));
	b.push_back(toCycle(b10));
	b.push_back(toCycle(b11));
	
	std::list<Cycles> generators;
	generators.push_back(a);
	generators.push_back(b);
	
	return Group(33, generators);
}

Group createProjectiveSpecialLinear35() {
	// (1 2 6)(3 4 5)(8 12 16 11 27 28)(9 17 20 10 22 24)(13 21 15 31 23 29)(14 26 19 30 18 25)
	const boost::array<unsigned long, 3> a1 = {1, 2, 6};
	const boost::array<unsigned long, 3> a2 = {3, 4, 5};
	const boost::array<unsigned long, 6> a3 = {8, 12, 16, 11, 27, 28};
	const boost::array<unsigned long, 6> a4 = {9, 17, 20, 10, 22, 24};
	const boost::array<unsigned long, 6> a5 = {13, 21, 15, 31, 23, 29};
	const boost::array<unsigned long, 6> a6 = {14, 26, 19, 30, 18, 25};
	Cycles a;
	a.push_back(toCycle(a1));
	a.push_back(toCycle(a2));
	a.push_back(toCycle(a3));
	a.push_back(toCycle(a4));
	a.push_back(toCycle(a5));
	a.push_back(toCycle(a6));
	
	// (1 3 5 4 6)(8 13 18 23 28)(9 19 29 14 24)(10 25 15 30 20)(11 31 26 21 16)
	const boost::array<unsigned long, 5> b1 = {1, 3, 5, 4, 6};
	const boost::array<unsigned long, 5> b2 = {8, 13, 18, 23, 28};
	const boost::array<unsigned long, 5> b3 = {9, 19, 29, 14, 24};
	const boost::array<unsigned long, 5> b4 = {10, 25, 15, 30, 20};
	const boost::array<unsigned long, 5> b5 = {11, 31, 26, 21, 16};
	Cycles b;
	b.push_back(toCycle(b1));
	b.push_back(toCycle(b2));
	b.push_back(toCycle(b3));
	b.push_back(toCycle(b4));
	b.push_back(toCycle(b5));
	
	// (1 4 5 6)(8 21 18 16)(9 30 29 20)(10 14 15 24)(11 23 26 28)(12 17 27 22)(13 31)(19 25)
	const boost::array<unsigned long, 4> c1 = {1, 4, 5, 6};
	const boost::array<unsigned long, 4> c2 = {8, 21, 18, 16};
	const boost::array<unsigned long, 4> c3 = {9, 30, 29, 20};
	const boost::array<unsigned long, 4> c4 = {10, 14, 15, 24};
	const boost::array<unsigned long, 4> c5 = {11, 23, 26, 28};
	const boost::array<unsigned long, 4> c6 = {12, 17, 27, 22};
	const boost::array<unsigned long, 2> c7 = {13, 31};
	const boost::array<unsigned long, 2> c8 = {19, 25};
	Cycles c;
	c.push_back(toCycle(c1));
	c.push_back(toCycle(c2));
	c.push_back(toCycle(c3));
	c.push_back(toCycle(c4));
	c.push_back(toCycle(c5));
	c.push_back(toCycle(c6));
	c.push_back(toCycle(c7));
	c.push_back(toCycle(c8));
	
	// (1 7 27 6)(2 11)(3 9 22 15)(4 10 12 19)(5 8 17 23)(13 25 18 14)(16 31)(20 30 24 29)
	const boost::array<unsigned long, 4> d1 = {1, 7, 27, 6};
	const boost::array<unsigned long, 2> d2 = {2, 11};
	const boost::array<unsigned long, 4> d3 = {3, 9, 22, 15};
	const boost::array<unsigned long, 4> d4 = {4, 10, 12, 19};
	const boost::array<unsigned long, 4> d5 = {5, 8, 17, 23};
	const boost::array<unsigned long, 4> d6 = {13, 25, 18, 14};
	const boost::array<unsigned long, 2> d7 = {16, 31};
	const boost::array<unsigned long, 4> d8 = {20, 30, 24, 29};
	Cycles d;
	d.push_back(toCycle(d1));
	d.push_back(toCycle(d2));
	d.push_back(toCycle(d3));
	d.push_back(toCycle(d4));
	d.push_back(toCycle(d5));
	d.push_back(toCycle(d6));
	d.push_back(toCycle(d7));
	d.push_back(toCycle(d8));
	
	std::list<Cycles> generators;
	generators.push_back(a);
	generators.push_back(b);
	generators.push_back(c);
	generators.push_back(d);
	
	return Group(31, generators);
}