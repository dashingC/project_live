#pragma once
#include <vector>
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>   
#include <functional>
#include <queue>
#include <mutex>
#include "Event.h"
// #include "PipeEvent.h"
// #include "TimingWheel.h"
/*
    IO就绪事件监听
    IO事件处理
    事件管理
*/
namespace tmms
{
namespace network
    {
        using EventPtr = std::shared_ptr<Event>;
        using Func = std::function<void()>;

        class EventLoop
        {
        public:
            EventLoop();
            ~EventLoop();

            void Loop();
            void Quit();

            void AddEvent(const EventPtr &event);
            void DelEvent(const EventPtr &event);

            bool EnableEventWriting(const EventPtr &event, bool enable);
            bool EnableEventReading(const EventPtr &event, bool enable);

            // 新增任务队列功能
            /*
                EventLoop提供执行任务的功能
                EventLoop执行一个任务有两种情况：
                    1. 调用方所在线程跟EventLoop所在线程是同一个线程，则直接执行
                    2. 调用方所在线程跟EventLoop所在线程不是同一个线程，把任务进队，由Loop去执行
                任务队列需要加锁
            */
            void AssertInLoopThread();
            bool IsInLoopThread() const;
            void RunInLoop(const Func &f);
            void RunInLoop(Func &&f);

            // 时间轮功能
            // void InsertEntry(uint32_t delay, EntryPtr entrPtr); 
            // void RunAfter(double delay, const Func &cb);
            // void RunAfter(double delay, Func &&cb);
            // void RunEvery(double inerval, const Func &cb);
            // void RunEvery(double inerval, Func &&cb);
        private:
            bool looping_{false};
            int epoll_fd_{-1};
            std::vector<struct epoll_event> epoll_events_;
            std::unordered_map<int, EventPtr> events_;
        
            // 任务队列私有
            // void RunFunctions();
            // void WakeUp();
            // std::queue<Func> functions_;
            // std::mutex lock_;
            // PipeEventPtr pipe_event_;

            // // 时间轮
            // TimingWheel wheel_;
        };
    }
}
//EventLoop在自己的线程中不断循环，等待 epoll 返回的就绪事件，并将这些事件分发给对应的 Event 处理器
//负责监听和分发事件，管理事件的生命周期，并提供任务队列功能