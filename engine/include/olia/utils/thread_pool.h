#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <memory>
#include <type_traits>

namespace Olia
{
    class ThreadPool
    {
    public:
        ThreadPool(size_t threads = std::thread::hardware_concurrency());
        ~ThreadPool();

        template<class F, class... Args>
        auto Enqueue(F&& f, Args&&... args) 
            -> std::future<typename std::invoke_result<F, Args...>::type>;

    private:
        // Need to keep track of threads so we can join them
        std::vector<std::thread> m_Workers;
        // The task queue
        std::queue<std::function<void()>> m_Tasks;
        
        // Synchronization
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        bool m_Stop;
    };

    // Constructor
    inline ThreadPool::ThreadPool(size_t threads)
        : m_Stop(false)
    {
        if (threads == 0) threads = 1; // fallback
        for(size_t i = 0; i<threads; ++i)
            m_Workers.emplace_back(
                [this]
                {
                    for(;;)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                            this->m_Condition.wait(lock,
                                [this]{ return this->m_Stop || !this->m_Tasks.empty(); });
                            if(this->m_Stop && this->m_Tasks.empty())
                                return;
                            task = std::move(this->m_Tasks.front());
                            this->m_Tasks.pop();
                        }

                        task();
                    }
                }
            );
    }

    // Add new work item to the pool
    template<class F, class... Args>
    auto ThreadPool::Enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);

            // Don't allow enqueueing after stopping the pool
            if(m_Stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            m_Tasks.emplace([task](){ (*task)(); });
        }
        m_Condition.notify_one();
        return res;
    }

    // Destructor joins all threads
    inline ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }
        m_Condition.notify_all();
        for(std::thread &worker: m_Workers)
            if (worker.joinable())
                worker.join();
    }
}
