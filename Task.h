#include <boost/shared_ptr.hpp>
#include <boost/thread/future.hpp>

#ifndef TASK_H
#define TASK_H

/**
 * A task is an adapter for a boost::packaged_task so that it can be used with the boost::asio package.
 */
template <class Ret>
class Task {
	// Task is a wrapper around a boost::shared_ptr<task_type>, as boost::asio::io_service::post()
	// requires that the input functor be copyable, which boost::packaged_task<Ret> isn't.
public:
	typedef typename boost::packaged_task<Ret> task_type;
	typedef typename boost::shared_future<Ret> future_type;
	
	/**
	 * Constructs a new Task from an existing task pointer.  Good for functors that aren't copyable.
	 */
	explicit Task(const boost::shared_ptr<task_type>& task_) : task(task_), future(task->get_future()) {}
	
	/**
	 * Constructs a new Task from an ordinary functor.
	 *
	 * @param <CopyableFunctor> The type of the functor.  This type must be copyable, and its operator()() must return a
	 *							value that's convertible to Ret.
	 */
	template <class CopyableFunctor>
	explicit Task(const CopyableFunctor& fn) : task(new task_type(fn)), future(task->get_future()) {}
	
	void operator()() const { (*task)(); }
	future_type get_future() const { return future; }
private:
	boost::shared_ptr<task_type> task;
	boost::shared_future<Ret> future;
};

#endif
