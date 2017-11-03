#pragma once
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <atomic>
#include <thread>

namespace xnet
{

class Thread
{
	using ThreadFunc = std::function<void()>;
	using ThreadPtr = std::shared_ptr<std::thread>;
public:
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;

	explicit Thread(const ThreadFunc& func, const std::string& name = std::string());
	explicit Thread(ThreadFunc&& func, const std::string& name = std::string());
	~Thread();

	void start();
	void join();

	bool started() { return started_; }
	std::thread::id tid() { return pThread_->get_id(); };
	const std::string& name() { return name_; }
	static int numCreated() { return numCreated_; }

private:
	void setDefaultName(const std::string&);

	std::string				name_;
	bool					started_;
	bool					joined_;
	ThreadFunc				func_;
	ThreadPtr				pThread_;

	static std::atomic<int> numCreated_;
};

}