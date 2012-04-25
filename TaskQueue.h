#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/thread.hpp>

#include "Task.h"

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

/**
 * Implementation of the thread-pool task queue paradigm.
 *
 * This paradigm is used in the parallel evaluation of GInvariants during discriminator construction.
 */
class TaskQueue {
public:
	TaskQueue(std::size_t numThreads = boost::thread::hardware_concurrency()) : work(io_service) {
		std::cerr << "Creating task queue with " << numThreads << " threads" << std::endl;
		for (std::size_t i = 0; i < numThreads; ++i) {
			// Create threads for thread pool
			threads.create_thread(boost::bind(&TaskQueue::run, this));
		}
	}
	
	~TaskQueue() {
		io_service.stop();
		threads.join_all();
	}
	
	/**
	 * Posts a new task to the worker threads.
	 */
	template <class Ret>
	boost::shared_future<Ret> schedule(const Task<Ret>& task) {
		io_service.post(task);
		return task.get_future();
	}
private:
	boost::asio::io_service io_service;
	boost::asio::io_service::work work;
	boost::thread_group threads;
	
	void run() { io_service.run(); }
};

#endif
