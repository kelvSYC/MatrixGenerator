#include "Pruner.h"

#include <boost/optional.hpp>
#include <boost/multi_array.hpp>

#ifndef MatrixGenerator_TablePruner_h
#define MatrixGenerator_TablePruner_h

class GInvariant;
class KMStrategy;

/* The following macros suppress the concurrent evaluation of cells in the table, as well as the dependency search
 * if the GInvariant type can spawn dependencies.
 */
//#define MATRIXGENERATOR_NO_CONCURRENT_EVALUATE
#define MATRIXGENERATOR_NO_DEPENDENCY_GRAPH

class TablePrunerData {
	friend class TablePruner;		// Only TablePruner can create instances
	
	// In C++11, the last function parameter can (and should) be instead a template arg; both are used to 
	// prevent accidental instantiation by a non-discriminator type.
	// glibal ::isTrivial call is needed as we have a member function of the same name
	template <class DiscriminatorType>
	explicit TablePrunerData(const boost::shared_ptr<DiscriminatorType>& _discriminator,
					typename boost::enable_if<isDiscriminator<DiscriminatorType> >::type* = 0) :
		discriminator(_discriminator), trivial(::isTrivial<DiscriminatorType>::value) {}
public:
	// Explicit "default" copy constructor, as the template constructor removes the general one
	TablePrunerData(const TablePrunerData& other) : discriminator(other.discriminator), trivial(other.trivial) {}
	
	boost::shared_ptr<GInvariant> getDiscriminator() const { return discriminator; }
	
	/**
	 * Returns whether the discriminator is trivial.  Specifically, if this returns false, then
	 * the discriminator can be safely downcast to a boost::shared_ptr<Discriminator>.
	 */
	bool isTrivial() const { return trivial; }
private:
	boost::shared_ptr<GInvariant> discriminator;
	bool trivial;
};

class TablePruner : public Pruner {
	typedef boost::multi_array<unsigned long, 2> Table;
	typedef Table::array_view<1>::type TableColumn;
	
	void addGInvariant(const boost::shared_ptr<GInvariant>& fn);
public:
	/**
	 * Creates a new TablePruner for k-subsets.
	 *
	 * @param orbitReps The orbit representatives for (k-1)-subsets
	 * @param strategy The GInvariant generation strategy
	 * @param prunerData Auxilliary data returned from the pruner for (k-1)-subsets.  If this is of type TablePrunerData,
	 *					 then the data is fed to the strategy.  All other types of data are ignored.
	 */
	TablePruner(const Group& G, unsigned int k, unsigned long rho, const std::vector<Subset>& orbitReps, const boost::shared_ptr<KMStrategy>& strategy, const boost::any& prunerData);
	
	void prune();
	
	std::vector<Subset> getNewReps();
	boost::any getNewData();
	size_t getColumn(const Subset& candidate);			// TODO - need a way to generate KM matrix without it
private:
	boost::shared_ptr<KMStrategy> strategy;
	boost::optional<TablePrunerData> prunerData;
	
	// Scratch material
	Table F;
	bool ready;
	std::vector<boost::shared_ptr<GInvariant> > fns;
	
	unsigned int k;										// TODO - move to common superclass
	unsigned long rho;									// TODO - move to common superclass
		
	// Output cache
	void initOutputs();
	boost::optional<std::vector<Subset> > newReps;
	boost::optional<TablePrunerData> newPrunerData;
	boost::optional<std::map<Subset, size_t> > candidateMap;
};

#endif
