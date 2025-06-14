#include "EventLoopThreadPool.h"
#include <pthread.h>

using namespace tmms::network;
namespace
{
    void bind_cpu(std::thread &t, int n)
    {
        // 1. 创建一个CPU核心的集合
        cpu_set_t cpu;
        // 2. 将集合清空
        CPU_ZERO(&cpu);
        // 3. 将编号为 n 的CPU核心加入到集合中
        CPU_SET(n, &cpu);
        // 4. 将线程 t 绑定到上述CPU核心集合中
        pthread_setaffinity_np(t.native_handle(), sizeof(cpu), &cpu);
    }
}

EventLoopThreadPool::EventLoopThreadPool(int thread_num, int start, int cpus)
{

    if (thread_num <= 0)
    {
        thread_num = 1;
    }

    for (int i = 0; i < thread_num; i++)
    {
        // emplace_back 在 vector 的末尾直接构造一个新的元素。
        // 在这里，它将刚刚创建的 std::shared_ptr<EventLoopThread> 添加到 threads_ 数组的尾部。
        // std::make_shared<EventLoopThread>():这行代码首先在堆上创建一个新的 EventLoopThread 对象。
        //EventLoopThread 的构造函数会立即启动一个新的子线程，这个子线程将准备运行一个 EventLoop
        threads_.emplace_back(std::make_shared<EventLoopThread>());
        if (cpus > 0)
        {
            int n = (start + i) % cpus;
            // 调用匿名命名空间中的辅助函数 bind_cpu，将底层的 std::thread 对象和
            // 我们计算出的核心编号 n 作为参数传进去，完成线程与CPU核心的绑定。
            bind_cpu(threads_.back()->Thread(), n);
        }
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

// 获取线程池中所有 EventLoop 的指针
std::vector<EventLoop *> EventLoopThreadPool::GetLoops() const
{

    std::vector<EventLoop *> results;
    for (auto &t : threads_)
    {
        // t是一个线程池中的一个 EventLoopThread 的智能指针
        // 调用 t->Loop() 获取 EventLoop 的指针，并将其添加到 results 中
        results.emplace_back(t->Loop());
    }
    return results;
}

// 以轮询的方式从线程池中获取下一个可用的 EventLoop
EventLoop *EventLoopThreadPool::GetNextLoop()
{

    int index = loop_index_;
    loop_index_++;
    return threads_[index % threads_.size()]->Loop();
}

// 返回线程池中配置的线程数量。
size_t EventLoopThreadPool::Size()
{
    return threads_.size();
}

// 启动线程池中的所有 EventLoop 线程。
void EventLoopThreadPool::Start()
{
    for (auto &t : threads_)
    {
        // 会启动子线程中的 EventLoop，并阻塞等待，直到那个 EventLoop 完成了内部的 promise/future 同步后，
        // Run() 方法才会返回。
        t->Run();
    }
}