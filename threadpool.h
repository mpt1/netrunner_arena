#pragma once

#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool;

class Worker
{
public:
    explicit Worker(ThreadPool& pool) :pool(pool){}

	void operator()();
private:
	ThreadPool& pool;
};

class ThreadPool
{
public:
	explicit ThreadPool(std::size_t size)
		: stop(false)
	{
		for (std::size_t i = 0; i < size; ++i)
		{
			workers.emplace_back(Worker(*this));
		}
	}

	~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			stop = true;
		}

		condition.notify_all();

		for (auto& worker : workers)
		{
			worker.join();
		}
	}

	template <typename F>
	void enque(F function);

private:
	friend class Worker;

	std::vector<std::thread> workers;
	std::deque<std::function<void()>> tasks;

	std::mutex mutex;
	std::condition_variable condition;
	bool stop;
};

template<typename F>
void ThreadPool::enque(F function)
{
	{
		std::unique_lock<std::mutex> lock(mutex);

		if (stop)
		{
			throw std::runtime_error("Enqueue on stopped ThreadPool");
		}

		tasks.push_back(std::function<void()>(function));
	}

	condition.notify_one();
}

inline void Worker::operator()()
{
	std::function<void()> task;
	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(pool.mutex);

			while (!pool.stop && pool.tasks.empty()) {
				pool.condition.wait(lock);
			}

			if (pool.stop && pool.tasks.empty()) {
				return;
			}

			task = pool.tasks.front();
			pool.tasks.pop_front();
		}

		task();
	}
}
