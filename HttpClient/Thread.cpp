#include <stdio.h>
#include "Thread.h"

using namespace xnet;

std::atomic<int> Thread::numCreated_ = 0;

Thread::Thread(const ThreadFunc& func, const std::string& name):
	func_(func),
	name_(name),
	started_(false),
	joined_(false)
{
	numCreated_++;
	if (name_.empty())
	{
		char buf[32];
		_snprintf_s(buf, sizeof buf, "Thread%d", numCreated_);
		name_ = buf;
	}
}

Thread::Thread(ThreadFunc&& func, const std::string& name):
	func_(std::move(func)),
	name_(name),
	started_(false),
	joined_(false)
{
	numCreated_++;
	if (name_.empty())
	{
		char buf[32];
		_snprintf_s(buf, sizeof buf, "Thread%d", numCreated_);
		name_ = buf;
	}
}

Thread::~Thread()
{
	if (started_ && !joined_)
	{
		pThread_->detach();
	}
}

void Thread::start()
{
	started_ = true;
	pThread_ = std::make_shared<std::thread>(func_);
}

void Thread::join()
{
	joined_ = true;
	pThread_->join();
}