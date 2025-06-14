#include "EventLoopThread.h"

using namespace tmms::network;

// thread_(...) 部分，意思是在构造 EventLoopThread 对象时，直接调用 thread_ 成员的构造函数来创建它
// [this]：捕获当前对象的 this 指针。在 Lambda 的函数体 {} 内部，你可以像在类的普通成员函数里一样，
// 访问这个类的所有成员变量和成员函数。这里std::thread 接收到这个 Lambda 后，立即创建一个新的子线程。
// 这个新创建的子线程开始执行 Lambda 的函数体 {} 里的代码。
// StartEventLoop() 的核心任务可以概括为：在一个全新的线程中，完成 EventLoop 的创建、与主线程的同步、运行以及最终的清理工作。
EventLoopThread::EventLoopThread() : thread_([this]()
                                             { StartEventLoop(); })
{
}

//析构函数
EventLoopThread::~EventLoopThread()
{
    Run();
    if (loop_)
    {
        loop_->Quit();
    }

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void EventLoopThread::Run()
{
    // 保证 EventLoopThread 的启动逻辑（设置 running_ = true、通知子线程 condition_.notify_all()）只发生一次。
    std::call_once(once_, [this]()
                   {
        {
            // 主线程在这里获取了同一个锁 lock_
            std::lock_guard<std::mutex> lk(lock_);
            running_ = true;
            // 通知等待在 condition_ 上的线程，表示 EventLoopThread 已经准备好开始运行了
            condition_.notify_all();
        }
         // 现代C++中用于线程间一次性值传递或同步的工具
         // std::promise：一个线程（生产者）做出承诺，表示“我未来会给你一个值”。
         // std::future：另一个线程（消费者）拿到了这个承诺的“凭证”，可以通过这个凭证在未来获取那个值。
        auto f = promise_loop.get_future();

        // 这是一个阻塞调用。主线程会在这里暂停执行，陷入等待，直到“承诺”被兑现。
        f.get(); });
}

EventLoop *EventLoopThread::Loop() const
{
    return loop_;
}

void EventLoopThread::StartEventLoop()//这个是子线程执行的
{
    // 步骤 1: 在新线程的栈上创建 EventLoop 实例
    EventLoop loop;

    // 步骤 2: 等待主线程将running设置为true
    std::unique_lock<std::mutex> lk(lock_);
    condition_.wait(lk, [this]()
                    { return running_; });
    
    // 步骤 3: 将新创建的 EventLoop 实例指针赋值给成员变量 loop_
    loop_ = &loop;// 最关键的一点是：将子线程栈上的loop对象的地址，写入到共享的成员变量loop_中
    // 把值 1 存入了与 promise_loop 关联的共享状态中。
    // 这个动作会立刻唤醒并解除正在 f.get() 处等待的主线程。主线程的 f.get() 会返回存入的值（虽然代码里没有接收返回值）。
    // 然后主线程继续向下执行。
    promise_loop.set_value(1);
    // 步骤 4: 启动事件循环，进入阻塞的事件处理状态
    loop.Loop();
    // 步骤 5: 循环结束后，进行清理
    loop_ = nullptr;
}

std::thread &EventLoopThread::Thread()
{
    return thread_;
}