#include "Group.h"
#include "Task.h"

#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/thread/future.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef GINVARIANT_H
#define GINVARIANT_H

class GInvariantEvaluationTask;

/**
 * A G-invariant function is a function that is fixed by every element in G.
 *
 * This is an abstract base class; specific constructions are detailed in subclasses.
 *
 * Subclasses must implement operator==(), which must call equals() (which may be overridden).
 * Subclasses must also implement hash() in a manner that conforms to the Boost::Hash standard.
 */
class GInvariant {
public:
	typedef boost::shared_ptr<GInvariant> ptr;
	
	GInvariant(const Group& _G) : G(_G.shared_from_this()) {}
	virtual ~GInvariant() {}
	
	const Group& getGroup() const { return *G; }
	
	/**
	 * Polymorphic equality function.
	 */
	virtual bool equals(const GInvariant& rhs) const;
	
	/**
	 * Polymorphic hash function.  Must be implemented in a manner conforming to the Boost::Hash standard.
	 */
	virtual std::size_t hash() const = 0;
	
	/**
	 * Evaluates this G-invariant function on the input subset.
	 */
	virtual unsigned long evaluate(const Subset& B) const = 0;
	
	/**
	 * Returns whether the result of the function evaluated over the input subset has previously been calculated.
	 * This method is only used in parallel execution of GInvariants when organizing the task threads.
	 */
	virtual bool hasCachedResult(const Subset& B) const = 0;
	
	/**
	 * Returns a list of uncached evaluation task that a call to evaluate(B) will also be forced to compute.
	 * This method is only used in parallel execution of GInvariants when organizing the task threads.
	 *
	 * The default implementation of this is to return an empty list.
	 */
	virtual std::deque<GInvariantEvaluationTask> getDependents(const Subset& B) const { return std::deque<GInvariantEvaluationTask>(); }
protected:
	boost::shared_ptr<const Group> G;
};

inline std::size_t hash_value(const GInvariant& fn) { return fn.hash(); }

/**
 * This functor simply evaluates a given GInvariant with an input subset.  It is used for the
 * concurrent evaluation of GInvariant functions.
 * 
 * To create the actual task, call the static create() method and pass in the arguments.
 */
class GInvariantEvaluationTask {
	GInvariant::ptr fn;
	Subset B;
	
public:
	// Hack to make GInvariantEvaluationTask work with Graph, which requires vertices to be a bit more substantial...
	GInvariantEvaluationTask() {}
	
	GInvariantEvaluationTask(const GInvariant::ptr& _fn, const Subset& _B) : fn(_fn), B(_B) {}
	
	GInvariant::ptr getFn() const { return fn; }
	Subset getInput() const { return B; }
	
	unsigned long operator()() const { return fn->evaluate(B); }
	
	std::deque<GInvariantEvaluationTask> getDependents() const { return fn->getDependents(B); }
	Task<unsigned long> package() const { return Task<unsigned long>(*this); }
	
	bool operator==(const GInvariantEvaluationTask& rhs) const { return fn->equals(*(rhs.fn)) && B == rhs.B; }
	bool operator!=(const GInvariantEvaluationTask& rhs) const { return !(*this == rhs); }
};

std::size_t hash_value(const GInvariantEvaluationTask& task);

/* ********************************************************************************************************************* */
// Traits classes for GInvariants
// These are classes representing properties of GInvariants that are known at compile-time.  Each subclass would specialize
// these templates as appropriate.

template <class GInvariantType> struct cachesResults;		// Whether the type employs an evaluation cache
template <class GInvariantType> struct isDiscriminator;		// Whether the type represents a discriminator
template <class GInvariantType> struct isSizeIndependent;	// Whether the type works on multiple input sizes
template <class GInvariantType> struct spawnsDependents;	// Whether the type needs to evaluate other GInvariants
template <class GInvariantType> struct isTrivial;			// Whether the type is a placeholder (eg. TrivialDiscriminator)

/**
 * Traits class used by GInvariant and subclasses.
 */
template <class GInvariantType>
struct GInvariantTraits {
	// All types employing evaluation caches must define an Evaluator class
	typedef typename boost::enable_if<cachesResults<GInvariantType>, typename GInvariantType::Evaluator>::type Evaluator;
};

#endif