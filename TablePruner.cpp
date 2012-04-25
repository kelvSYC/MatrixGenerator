#include <algorithm>

#include <boost/mem_fn.hpp>
#include <boost/throw_exception.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/thread.hpp>

#include "AdjacencyList.h"
#include "Discriminator.h"
#include "KMStrategy.h"
#include "TablePruner.h"
#include "TaskQueue.h"
#include "TrivialDiscriminator.h"

/**
 * Quick singleton wrapper for the task queue so we don't have to recreate it repeatedly.
 *
 * Thread-safe only in C++11 and under certain compilers in C++03, including clang and gcc.
 */
class ThreadPool : public boost::noncopyable {
	TaskQueue task_queue;
	
	ThreadPool() : task_queue() {}
public:
	static ThreadPool& getInstance() {
		static ThreadPool instance;
		return instance;
	}
	
	template <class Ret>
	boost::shared_future<Ret> schedule(const Task<Ret>& task) {
		return task_queue.schedule(task);
	}
};

/* *********************************************************************************************** */
/**
 * Task functor that populates the dependecies for a specific GInvariantEvaluationTask.
 */
class GraphPopulationTask {
	Graph<GInvariantEvaluationTask>& G;
	std::deque<GInvariantEvaluationTask> tasks;
	
	GraphPopulationTask(Graph<GInvariantEvaluationTask>& G_, const GInvariantEvaluationTask& startingTask) : G(G_), tasks() {
		tasks.push_back(startingTask);
	}
public:
	/**
	 * Adds the task and its dependents to the graph.
	 */
	void operator()() {
		while (!tasks.empty()) {
			GInvariantEvaluationTask task = tasks.front();
			tasks.pop_front();
			
			// TODO - check to see if the dependents were not previously added
			G.addVertex(task);
			std::deque<GInvariantEvaluationTask> dependents = task.getDependents();
			for (std::deque<GInvariantEvaluationTask>::const_iterator it = dependents.begin(); it != dependents.end(); ++it) {
				G.addEdge(task, *it);
				tasks.push_back(*it);
			}
		}
	}
	
	static Task<void> create(Graph<GInvariantEvaluationTask>& G, const GInvariantEvaluationTask& startingTask) {
		typedef boost::packaged_task<void> task_type;
		typedef boost::shared_ptr<task_type> task_ptr;
		
		GraphPopulationTask task(G, startingTask);
		task_ptr ptr = task_ptr(new task_type(task));
		return Task<void>(ptr);
	}
};

/* *********************************************************************************************** */
TablePruner::TablePruner(const Group& G, unsigned int _k, unsigned long _rho, const std::vector<Subset>& orbitReps, const boost::shared_ptr<KMStrategy>& _strategy, const boost::any& _prunerData) :
	Pruner(G, rho, DefaultCandidateGenerator(G.getNumPoints(), orbitReps)), k(_k), rho(_rho), strategy(_strategy) {
	if (rho == 1) {
		// The program should never reach here, as we have assumed rho > 1.  But just in case...
		ready = true;
	} else {
		// Any useful previous data?
		if (_prunerData.type() == typeid(TablePrunerData)) {
			prunerData = boost::any_cast<TablePrunerData>(_prunerData);
		}
		
		F.resize(boost::extents[0][getCandidates().size()]);
	}
}

void TablePruner::addGInvariant(const boost::shared_ptr<GInvariant>& fn) {
	if (ready) return;	// Do nothing if we already have full discrimination;
	
	const std::vector<Subset>& candidates = getCandidates();
	size_t rowIdx = fns.size();									// Row index of the new function
	fns.push_back(fn);
	F.resize(boost::extents[fns.size()][candidates.size()]);	// Add new row to F
	
#ifdef MATRIXGENERATOR_NO_CONCURRENT_EVALUATE
	// NON-CONCURRENT EVALUATION
	for (size_t i = 0; i < candidates.size(); ++i) {
		F[rowIdx][i] = fn->evaluate(candidates[i]);
	}
#else	// MATRIXGENERATOR_NO_CONCURRENT_EVALUATE not defined
	// CONCURRENT EVALUATION
	// Chances are, you can't get one thread for every single evaluation, just because there are too
	// many evaluations to be done.  So, we have to create a thread pool.
	ThreadPool& task_queue = ThreadPool::getInstance();
	std::vector<boost::shared_future<unsigned long> > futures;
	futures.reserve(candidates.size());
	
#ifdef MATRIXGENERATOR_NO_DEPENDENCY_GRAPH
	// Create, package, and enqueue tasks
	for (std::vector<Subset>::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {
		GInvariantEvaluationTask evalTask(fn, *it);
		Task<unsigned long> task = evalTask.package();
		futures.push_back(task.get_future());
		task_queue.schedule(task);
	}
	std::cerr << candidates.size() << " evaluation tasks created" << std::endl;
#else
	// Find and sort dependencies
	Graph<GInvariantEvaluationTask> dependencyGraph;
	std::vector<boost::shared_future<void> > graphTaskFutures;
	graphTaskFutures.reserve(labels.size());
	for (std::vector<Subset>::const_iterator it = labels.begin(); it != labels.end(); ++it) {
		GInvariantEvaluationTask evalTask(fn, *it);
		
		// Create a GraphPopulationTask for each cell, and push it on the (idling) task execution pool
		Task<void> task = GraphPopulationTask::create(dependencyGraph, evalTask);
		boost::shared_future<void> future = task_queue.schedule(task);
		graphTaskFutures.push_back(future);
	}
	boost::wait_for_all(graphTaskFutures.begin(), graphTaskFutures.end());
	std::list<GInvariantEvaluationTask> sortedTasks = dependencyGraph.topological_sort();
	
	// Package and enqueue tasks
	for (std::list<GInvariantEvaluationTask>::const_iterator it = sortedTasks.begin(); it != sortedTasks.end(); ++it) {
		if (it->getFn()->equals(*fn)) {
			// We must package *it and also get its future
			for (int i = 0; i < labels.size(); ++i) {		// For putting the future in the right place
				if (labels[i].getLabel() == it->getInput()) {
					Task<unsigned long> task = it->package();
					boost::shared_future<unsigned long> future = task.get_future();
					
					futures.push_back(future);
					task_queue.schedule(task);
					break;
				}
			}
		} else {
			// We don't need the result of this, so we just package and schedule
			task_queue.schedule(it->package());
		}
	}
	std::cerr << sortedTasks.size() << " evaluation tasks created" << std::endl;
#endif	// MATRIXGENERATOR_NO_DEPENDENCY_GRAPH
	
	boost::wait_for_all(futures.begin(), futures.end());
	std::transform(futures.begin(), futures.end(), F[rowIdx].begin(), boost::mem_fn(&boost::shared_future<unsigned long>::get));
#endif	// MATRIXGENERATOR_NO_CONCURRENT_EVALUATE
	
	// Now, do we have a discriminator?  We do if and only if the number of distinct columns in F is the same
	// as rho.  That's what we find out next.
	std::set<TableColumn> columnSet;
	for (size_t i = 0; i < candidates.size(); ++i) {
		columnSet.insert(F[boost::indices[Table::index_range()][i]]);
	}
	std::cerr << columnSet.size() << "/" << rho << " orbit representatives found" << std::endl;
	if (columnSet.size() == rho) ready = true;
}

void TablePruner::prune() {
	std::vector<boost::shared_ptr<GInvariant> > initialFns;
	
	if (prunerData) {
		// Add in the initial functions, as predicted by the strategy
		initialFns = strategy->createInitialGInvariants(*G, k, *prunerData);
	}
	
	std::vector<boost::shared_ptr<GInvariant> >::const_iterator it = initialFns.begin();
	while (!ready) {
		if (it != initialFns.end()) {
			// We still have at least one inital function to try
			addGInvariant(*it++);
		} else {
			// We are out of initial functions, so we rely on our strategy to create a new function and
			// hopefully push the cartesian product to full discrimination
			addGInvariant(strategy->createNewGInvariant(*G, k));
		}
	}
}

void TablePruner::initOutputs() {
	newReps = std::vector<Subset>();
	newReps->reserve(rho);
	candidateMap = std::map<Subset, size_t>();
	
	if (rho == 1) {
		// The algorithm should bypass using pruners of any kind if rho == 1, but just in case...
		newReps->push_back(generateX(k));
		boost::shared_ptr<TrivialDiscriminator> discriminator(new TrivialDiscriminator(*G));
		newPrunerData = TablePrunerData(discriminator);
	} else {
		typedef std::vector<unsigned long> FrequencyVector;
		const std::vector<Subset>& candidates = getCandidates();
		
		// Convert the columns of the table to FrequencyVector
		std::set<FrequencyVector> columnsToProcess;
		std::vector<FrequencyVector> columns;
		columns.reserve(candidates.size());
		for (int i = 0; i < candidates.size(); ++i) {
			TableColumn column = F[boost::indices[Table::index_range()][i]];
			
			FrequencyVector colVector(column.begin(), column.end());
			columns.push_back(colVector);
			columnsToProcess.insert(colVector);
		}
		
		std::map<FrequencyVector, unsigned long> lookupTable;
		std::map<Subset, unsigned long> newCache;
		unsigned long nextIdx = 0;
		for (int i = 0; i < columns.size(); ++i) {
			FrequencyVector fv = columns[i];
			
			if (columnsToProcess.count(fv) != 0) {
				// candidates[i] is a new orbit representative (in lexicogrpahical order)
				newReps->push_back(candidates[i]);
				
				// Add new mapping to lookupTable
				lookupTable[fv] = nextIdx++;
				
				columnsToProcess.erase(columnsToProcess.find(fv));
			}
			
			// We also have it that the discriminator, when evaluated at candidates[i], returns lookupTable[fv].
			// Also, the minimum representative for candidates[i] is at newReps[lookupTable[fv]].
			unsigned long idx = lookupTable[fv];
			newCache[candidates[i]] = idx;
			(*candidateMap)[candidates[i]] = idx;
		}
		
		// Build the discriminator and store it in the prunerData
		boost::shared_ptr<Discriminator> discriminator(new Discriminator(*G, fns, lookupTable, newCache));
		newPrunerData = TablePrunerData(discriminator);
	}
}

std::vector<Subset> TablePruner::getNewReps() {
	if (!ready) boost::throw_exception(PrunerNotReady());
	if (!newReps) initOutputs();
	return *newReps;
}

boost::any TablePruner::getNewData() {
	if (!ready) boost::throw_exception(PrunerNotReady());
	if (!newPrunerData) initOutputs();
	return *newPrunerData;
}

size_t TablePruner::getColumn(const Subset& candidate) {
	if (!ready) boost::throw_exception(PrunerNotReady());
	if (!candidateMap) initOutputs();
	if (candidateMap->count(candidate) == 0) {
		// If it isn't in the candidate map (it shouldn't be in practice...)
		// Evaluate the discriminator to get the result
		(*candidateMap)[candidate] = newPrunerData->getDiscriminator()->evaluate(candidate);
	}
	return (*candidateMap)[candidate];
}