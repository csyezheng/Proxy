//
// Created by csyezheng on 11/22/16.
//

#ifndef PROXY_THREADPOOL_H
#define PROXY_THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <future>
#include <functional>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>


namespace Proxy
{
    class ThreadPool
    {
    public:
        ThreadPool(size_t);
        ~ThreadPool();
        template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args)
            -> std::function<typename std::result_of<F(Args...)>::type>;
    private:
        void task_thread();
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> task_queue;
        std::mutex queue_mutex;
        std::condition_variable cond_var;
        bool done;
    };
}




#endif //PROXY_THREADPOOL_H
