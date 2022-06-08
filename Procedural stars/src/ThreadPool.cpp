#include "ThreadPool.h"

ThreadPool::ThreadPool(std::uint32_t numThreads) {
    std::uint32_t maxThreads = std::thread::hardware_concurrency() - sm_threadsOccupied;
    m_numThreads = std::clamp(numThreads, 0U, maxThreads);
    sm_threadsOccupied += m_numThreads;
    m_threads.resize(m_numThreads);
    for (uint32_t i = 0; i < m_numThreads; i++) {
        m_threads[i] = std::thread(&ThreadPool::ThreadLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queueLock);
        m_stop = true;
    }
    m_mutexCondition.notify_all();
    for (std::thread& active_thread : m_threads) {
        active_thread.join();
    }
    m_threads.clear();
    sm_threadsOccupied -= m_numThreads;
}

void ThreadPool::AddJob(const std::function<void()>& job) {
    {
        std::unique_lock<std::mutex> lock(m_queueLock);
        m_jobQueue.push(job);
    }
    m_mutexCondition.notify_one();
}

void ThreadPool::ThreadLoop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(m_queueLock);
            m_mutexCondition.wait(lock, [this] { return !m_jobQueue.empty() || m_stop;});
            if (m_stop)
                return;
            job = m_jobQueue.front();
            m_jobQueue.pop();
        }
        job();
    }
}

std::uint32_t ThreadPool::sm_threadsOccupied = 0;