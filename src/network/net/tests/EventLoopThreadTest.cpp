#include "network/net/EventLoopThread.h"
#include "network/net/EventLoopThreadPool.h"
#include "network/net/EventLoop.h"
#include "base/TTime.h"
#include "network/net/PipeEvent.h"
#include <iostream>
#include <thread>

using namespace tmms::network;

// 创建一个EventLoopThread对象
EventLoopThread eventloop_thread;
std::thread th;

void TestEventLoopThread()
{

    // 主线程调用Run()，启动子线程并等待其初始化完成
    eventloop_thread.Run();
    // 主线程调用 eventloop_thread.Loop() 获取子线程中运行的 EventLoop 对象的指针
    EventLoop *loop = eventloop_thread.Loop();

    // 主线程使用获取到的 loop 指针进行操作
    if (loop)
    {
        std::cout << "loop:" << loop << std::endl;
        // 将 loop 指针传递给 PipeEvent 的构造函数，让 PipeEvent 知道它属于哪个循环
        PipeEventPtr pipe_event = std::make_shared<PipeEvent>(loop);
        // 调用 loop 的方法，将新的事件注册到子线程的 epoll 中
        loop->AddEvent(pipe_event);
        int64_t test = 12345;
        // 主线程通过 pipe_event 对象写入数据，触发子线程中的读事件
        pipe_event->Write((const char *)&test, sizeof(test));

        th = std::thread([&pipe_event]()
                         {
            
            while(1){
                std::this_thread::sleep_for(std::chrono::seconds(1));
                int64_t now = tmms::base::TTime::NowMS();
                pipe_event->Write((const char*)&now, sizeof(now));
            } }); // 子线程会阻塞在这里面

        // 主线程继续执行其他操作
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void TestEventLoopThreadPool()
{
    // 创建一个 EventLoopThreadPool 对象，2个线程，2个CPU
    EventLoopThreadPool pool(2, 0, 2);

    pool.Start();

    // // 测试用例1 线程
    // std::vector<EventLoop*> list = pool.GetLoops();
    // for( auto &e : list)
    // {
    //     std::cout << "loop:" << e <<std::endl;
    // }
    // EventLoop * loop = pool.GetNextLoop();
    // std::cout << "loop:" << loop <<std::endl;
    // loop = pool.GetNextLoop();
    // std::cout << "loop:" << loop <<std::endl;

    // // 测试用例2 线程池
    // std::cout << "主线程 id : " << std::this_thread::get_id() << std::endl;

    // std::vector<EventLoop *> loops = pool.GetLoops();

    // for (auto &e : loops)
    // {

    //     e->RunInLoop([&e]()
    //                  { std::cout << "loop:" << e << " 子线程 id : " << std::this_thread::get_id() << std::endl; });
    // }

    // 测试用例3 时间轮
    EventLoop *loop = pool.GetNextLoop();
    std::cout << "loop:" << loop << std::endl;
    loop->RunAfter(1, []()
                   { std::cout << "run after 1s now:" << tmms::base::TTime::Now() << std::endl; });
    loop->RunAfter(5, []()
                   { std::cout << "run after 5s now:" << tmms::base::TTime::Now() << std::endl; });
    loop->RunEvery(1, []()
                   { std::cout << "run every 1s now:" << tmms::base::TTime::Now() << std::endl; });
    loop->RunEvery(5, []()
                   { std::cout << "run every 5s now:" << tmms::base::TTime::Now() << std::endl; });
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, const char **argv)
{
    TestEventLoopThreadPool();
    // TestEventLoopThread();
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
