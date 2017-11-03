#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include "ThreadPool.h"
using namespace xnet;

ThreadPool::ThreadPool(const std::string& name) :
	name_(name),
	maxQueueSize_(0),
	running_(false)
{
}

ThreadPool::~ThreadPool()
{
	if (running_)
	{
		stop();
	}
}

void ThreadPool::stop()
{
	{
		MutexLockGuard lock(mutex_);
		running_ = false;
		notEmpty_.notify_all();
	}

	for (auto& thread : threads_)
	{
		thread->join();
	}
}

void ThreadPool::start(int numThreads)
{
	running_ = true;
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; i++)
	{
		ThreadPtr pThread = std::make_shared<Thread>(std::bind(&ThreadPool::runInThread, this));
		pThread->start();
		threads_.push_back(pThread);
	}
}

void ThreadPool::run(const Task& task)
{
	if (threads_.empty())
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_);
		while (isFull())
		{
			notFull_.wait(lock);
		}

		queue_.push_back(task);
		notEmpty_.notify_one();
	}
}

void ThreadPool::run(Task&& task)
{
	if(threads_.empty())
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_);
		while(isFull())
		{
			notFull_.wait(lock);
		}
		queue_.push_back(std::move(task));
		notEmpty_.notify_one();
	}
}

void ThreadPool::runInThread()
{
	try{
		while (running_)
		{
			Task task = take();
			if (task)
			{
				task();
			}
		}
	}
	catch(const std::exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", name_.data());
		fprintf(stderr, "reason: %s\n", ex.what());
		abort();
	}
	catch(...)
	{
		fprintf(stderr, "Unknown exception caught in ThreadPool %s\n", name_.data());
		throw;
	}
}

ThreadPool::Task ThreadPool::take()
{
	MutexLockGuard lock(mutex_);
	while (queue_.empty() && running_)
	{
		notEmpty_.wait(lock);
	}
	Task task;
	if (!queue_.empty())
	{
		task = queue_.front();
		queue_.pop_front();
		if (maxQueueSize_ > 0)
		{
			notFull_.notify_one();
		}
	}

	return task;
}

bool ThreadPool::isFull() const
{
	return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}
