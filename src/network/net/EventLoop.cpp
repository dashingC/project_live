#include "EventLoop.h"
#include "network/base/Network.h"
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "base/TTime.h"

using namespace tmms::network;

// 一个线程只需有一个事件循环最多
static thread_local EventLoop *t_local_eventloop = nullptr;

EventLoop::EventLoop() : epoll_fd_(::epoll_create(1024)), epoll_events_(1024)
{
    //这个线程已经有一个事件循环了
    if (t_local_eventloop)
    {
        NETWORK_ERROR << "EventLoop already exists in this thread!!!!!";
        exit(-1);
    }
    t_local_eventloop = this;
}

EventLoop::~EventLoop()
{
    Quit();
}

void EventLoop::Loop()
{
    // 步骤 1: 初始化循环控制变量
    // looping_ 是一个布尔成员变量，用于控制 while 循环的执行。
    // 在调用 Quit() 方法前，它一直为 true。
    looping_ = true;
     // 设置 epoll_wait 的超时时间为 1000 毫秒（1秒）。
    // 这意味着 epoll_wait 最多阻塞1秒，即使没有任何I/O事件，也会返回一次。
    int64_t timeout = 1000;
    while (looping_)
    {
        memset(&epoll_events_[0], 0x00, sizeof(struct epoll_event) * epoll_events_.size());

        // 步骤 2: 调用 epoll_wait，阻塞等待I/O事件
        // - epoll_fd_: epoll 实例的文件描述符，在 EventLoop 构造时创建。
        // - &epoll_events_[0]: 指向用于存储就绪事件的数组的指针。
        // - epoll_events_.size(): 数组的大小，告诉内核最多可以返回多少个事件。
        // - timeout: 最大阻塞时间。
        // - 返回值 (ret): 发生事件的文件描述符数量。如果超时则返回0，如果出错则返回-1。
        auto ret = ::epoll_wait(epoll_fd_,
                                (struct epoll_event *)&epoll_events_[0],
                                static_cast<int>(epoll_events_.size()),
                                timeout);
        if (ret >= 0) // 大于等于0表示调用成功（可能超时）
        {
            // 步骤3: 遍历所有就绪的事件
            for (int i = 0; i < ret; i++)
            {
                struct epoll_event &ev = epoll_events_[i];// 获取一个就绪事件
                if (ev.data.fd <= 0)// 安全检查：fd <= 0 是无效的
                {
                    NETWORK_ERROR << "epoll wait error(非法fd). fd:" << ev.data.fd;
                    continue;
                }
                //从 events_ 哈希表中查找 fd 对应的 Event 对象，是一个键值对
                auto iter = events_.find(ev.data.fd);
                if (iter == events_.end())// 如果找不到，说明该Event可能已被移除
                {
                    NETWORK_ERROR << "epoll wait error(查不到). fd:" << ev.data.fd;
                    continue;
                }
                EventPtr &event = iter->second;
               
                // 根据具体的事件类型，调用相应的回调函数
                // 1、事件出错
                if (ev.events & EPOLLERR)
                {

                    int error = 0;
                    socklen_t len = sizeof(error);
                    // 获取具体的套接字错误信息
                    getsockopt(event->Fd(), SOL_SOCKET, SO_ERROR, &error, &len);

                    event->OnError(strerror(error));// 调用 OnError 回调
                }
                // 2、连接被挂断 (对端关闭)，且当前没有可读数据
                else if ((ev.events & EPOLLHUP) && !(ev.events & EPOLLIN))
                {
                    event->OnClose();
                }
                // 3、可读事件或紧急数据事件
                else if (ev.events & (EPOLLIN | EPOLLPRI))
                {
                    event->OnRead();
                }
                // 4、可写事件
                else if (ev.events & EPOLLOUT)
                {
                    event->OnWrite();
                }
            }

            //// 步骤 4: 动态扩容事件数组
            // 如果本次返回的事件数量等于数组容量，说明数组可能太小了，
            // 下次可能还有更多事件没有被一次性取回。因此，将数组容量翻倍。
            if (ret == epoll_events_.size())
            {
                epoll_events_.resize(epoll_events_.size() * 2);
            }
            RunFunctions();
            int64_t now = tmms::base::TTime::NowMS();
            wheel_.OnTimer(now);
        }
        else if (ret < 0)
        {
            NETWORK_ERROR << "epoll wait error.error:" << errno;
        }
    }
}

void EventLoop::Quit()
{
    looping_ = false;
}

//增加事件
void EventLoop::AddEvent(const EventPtr &event)
{
    auto iter = events_.find(event->Fd());
    if (iter != events_.end())
    {
        return;
    }
    event->event_ |= kEventRead;
    events_[event->Fd()] = event;

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event->fd_, &ev);
}
//删除事件
void EventLoop::DelEvent(const EventPtr &event)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        return;
    }
    events_.erase(iter);

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event->fd_, &ev);
}
//使能写事件
bool EventLoop::EnableEventWriting(const EventPtr &event, bool enable)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        NETWORK_ERROR << "Event not found in event loop--fd:" << event->Fd();
        return false;
    }

    if (enable)
    {
        event->event_ |= kEventWrite;
    }
    else
    {
        event->event_ &= ~kEventWrite;
    }

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->fd_, &ev);
    return true;
}
//使能读事件
bool EventLoop::EnableEventReading(const EventPtr &event, bool enable)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        NETWORK_ERROR << "Event not found in event loop--fd:" << event->Fd();
        return false;
    }

    if (enable)
    {
        event->event_ |= kEventRead;
    }
    else
    {
        event->event_ &= ~kEventRead;
    }

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->fd_, &ev);
    return true;
}

// 任务队列函数
void EventLoop::AssertInLoopThread()
{
    if (!IsInLoopThread())
    {
        NETWORK_ERROR << "It is forbidden to run loop on other thread!!!! ";
        exit(-1);
    }
}

bool EventLoop::IsInLoopThread() const
{
    return t_local_eventloop == this;
}

void EventLoop::RunInLoop(const Func &f)
{
    if (IsInLoopThread())
    {
        f();
    }
    else
    {
        std::lock_guard<std::mutex> lk(lock_);
        functions_.push(f);

        WakeUp();
    }
}
void EventLoop::RunInLoop(Func &&f)
{
    if (IsInLoopThread())
    {
        f();
    }
    else
    {
        std::lock_guard<std::mutex> lk(lock_);
        functions_.push(std::move(f));

        WakeUp();
    }
}

void EventLoop::RunFunctions()
{
    std::lock_guard<std::mutex> lk(lock_);
    while (!functions_.empty())
    {
        auto &f = functions_.front();
        f();
        functions_.pop();
    }
}
void EventLoop::WakeUp()
{
    if (!pipe_event_)
    {
        pipe_event_ = std::make_shared<PipeEvent>(this);
        AddEvent(pipe_event_);
    }
    int tmp = 1;
    pipe_event_->Write((const char *)&tmp, sizeof(tmp));
}

// 时间轮功能
void EventLoop::InsertEntry(uint32_t delay, EntryPtr entrPtr)
{
    if (IsInLoopThread())
    {
        wheel_.InsertEntry(delay, entrPtr);
    }
    else
    {
        RunInLoop([this, delay, entrPtr]()
                  { wheel_.InsertEntry(delay, entrPtr); });
    }
}

void EventLoop::RunAfter(double delay, const Func &cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunAfter(delay, cb);
    }
    else
    {
        RunInLoop([this, delay, cb]()
                  { wheel_.RunAfter(delay, cb); });
    }
}

void EventLoop::RunAfter(double delay, Func &&cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunAfter(delay, cb);
    }
    else
    {
        RunInLoop([this, delay, cb]()
                  { wheel_.RunAfter(delay, cb); });
    }
}

void EventLoop::RunEvery(double inerval, const Func &cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunEvery(inerval, cb);
    }
    else
    {
        RunInLoop([this, inerval, cb]()
                  { wheel_.RunEvery(inerval, cb); });
    }
}

void EventLoop::RunEvery(double inerval, Func &&cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunEvery(inerval, cb);
    }
    else
    {
        RunInLoop([this, inerval, cb]()
                  { wheel_.RunEvery(inerval, cb); });
    }
}