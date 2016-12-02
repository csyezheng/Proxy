//
// Created by csyezheng on 11/22/16.
//

#include "ThreadPool.h"
#include <thread>
#include <mutex>

using namespace std;
using namespace Proxy;


ThreadPool::ThreadPool(size_t thread_nums): done(false)
{
    __try{
        for (auto i = 0; i != thread_nums; ++i)
        {
            threads.push_back(
                    thread(ThreadPool::task_thread, this));
        }
    }
    catch(...)
    {
        done = true;
        throw;
    }
}

ThreadPool::~ThreadPool()
{
    unique_lock<mutex> lock(queue_mutex);
    done = true;
    cond_var.notify_all();
    for (auto &task: threads)
        task.join();
}

template<typename F, typename... Args>
auto ThreadPool::enqueue(F &&f, Args &&args)
    -> std::future<typename result_of<F(Args...)>::type>
{
    using return_type = typename result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type>> (
            bind(forward<F>(f), forward<Args>(args)...)
    );

    future<return_type> ret = task->get_future();

    std::unique_lock<std::mutex> lock(queue_mutex);
    if (done)
        throw runtime_error("enqueue on stopped ThreadPool");
    task_queue.emplace([task](){ /*(*task)();*/ });

    cond_var.notify_one();
    return ret;
}

void ThreadPool::task_thread()
{
    while(!done)
    {
        function<void()> task;
        if (task_queue != nullptr)
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->cond_var.wait(lock,
                [this]{ return this->done || this->task_queue.empty();});
            if (done && task_queue.empty())
                return;
            task = std::move(task_queue.front());
            task_queue.pop();
            task();
        }
        else
        {
            this_thread::yield();
        }
    }
}