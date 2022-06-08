#pragma once
#include <mutex>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <cstdint>

class ThreadPool {
public:
	ThreadPool(std::uint32_t numThreads);
	~ThreadPool();
	void AddJob(const std::function<void()>& job);
private:
	void ThreadLoop();
	bool m_stop = false;
	std::uint32_t m_numThreads;
	std::vector<std::thread> m_threads;
	std::mutex m_queueLock;
	std::queue<std::function<void()>> m_jobQueue;
	std::condition_variable m_mutexCondition;

	static std::uint32_t sm_threadsOccupied;
};