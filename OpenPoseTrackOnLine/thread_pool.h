//
// Created by 高一奥 on 2025/6/11.
//

#ifndef OPENPOSETRACK_THREAD_POOL_H
#define OPENPOSETRACK_THREAD_POOL_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>
#include <functional>
#include <unordered_map>

template <typename T>
class SafeQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;

public:
    SafeQueue() {}
    SafeQueue(SafeQueue&& other) {}
    ~SafeQueue(){}

    bool empty(){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size(){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T& t){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(t);
    }

    bool dequeue(T& t){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty()) return false;
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
};

class ThreadPool
{
private:

    class ThreadWorker
    {
    private:
        ThreadPool* m_pool;
        int m_id;

    public:
        ThreadWorker(ThreadPool* pool, int id) :
                m_pool(pool),
                m_id(id){

        }

        void operator()(){

            {
                std::unique_lock<std::mutex> lock_threadId(m_pool->m_threadId_mutex);
                m_pool->m_thread_index[std::this_thread::get_id()] = m_id;
            }

            std::function<void()> func;
            bool dequeued;
            /*
            while(!m_pool->m_shutdown){
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_condition_mutex);
                    if(m_pool->m_queue.empty()) m_pool->m_condition_lock.wait(lock);
                    dequeued = m_pool->m_queue.dequeue(func);
                }
                if(dequeued) func();
            }
            */

            while(true){
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_condition_mutex);
                    m_pool->m_condition_lock.wait(lock, [this]() { return m_pool->m_shutdown || !m_pool->m_queue.empty();});
                    if(m_pool->m_shutdown && m_pool->m_queue.empty()) return;
                    if(!m_pool->m_queue.dequeue(func)) continue;
                }
                func();

            }
        }
    };

    SafeQueue<std::function<void()>> m_queue;
    bool m_shutdown;
    std::mutex m_condition_mutex;
    std::condition_variable m_condition_lock;
    std::vector<std::thread> m_threads;

    std::unordered_map<std::thread::id, int> m_thread_index;
    std::mutex m_threadId_mutex;



public:
    ThreadPool(int thread) :
            m_threads(std::vector<std::thread>(thread)),
            m_shutdown(false){
        init();
    }

    ~ThreadPool(){
        shutdown();
    }

    void init(){
        std::cout << "Threadpool is initialing!" << std::endl;
        for(int i = 0; i < m_threads.size(); ++i){
            m_threads.at(i) = std::thread(ThreadWorker(this, i));
        }
    }

    void shutdown(){
        m_shutdown = true;
        m_condition_lock.notify_all();
        for(auto& thread : m_threads){
            if(thread.joinable()) thread.join();
        }
    }

    int getThreadIndex(std::thread::id threadId){
        return m_thread_index[threadId];
    }

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> void_func = [task_ptr]() { (*task_ptr)(); };
        m_queue.enqueue(void_func);
        m_condition_lock.notify_one();
        return task_ptr->get_future();
    }

    template <typename F, typename... Args>
    void submit_void(F&& f, Args&&... args){
        std::function<void()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> func_void = [task_ptr]() { (*task_ptr)();};

        m_queue.enqueue(func_void);
        m_condition_lock.notify_one();
    }
};


#endif //OPENPOSETRACK_THREAD_POOL_H
