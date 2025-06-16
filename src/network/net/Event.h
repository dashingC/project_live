#pragma once
#include <string>
#include <sys/epoll.h>
#include <memory>

namespace tmms
{
    namespace network
    {
        class EventLoop;
        const int kEventRead = (EPOLLIN | EPOLLPRI | EPOLLET);
        const int kEventWrite = (EPOLLOUT | EPOLLET);

        // 关键点：当 Event 类写下 public std::enable_shared_from_this<Event> 时，
        // 它就为自己以及所有从它派生的子类定下了一个规则：
        // shared_from_this() 函数将返回一个 std::shared_ptr<Event>
        class Event:public std::enable_shared_from_this<Event>
        {
            friend class EventLoop;
        public:
            Event(EventLoop *loop);
            Event(EventLoop *loop, int fd);
            virtual ~Event();

            //question：为什么这里要定义成一个虚函数
            virtual void OnRead() {};
            virtual void OnWrite() {};
            virtual void OnClose() {};
            virtual void OnError(const std::string &msg) {};
            
            bool EnableWriting(bool enable);
            bool EnableReading(bool enable);

            int Fd() const;//返回文件描述符
            void Close();

        protected:
            EventLoop * loop_{nullptr};
            int fd_{-1};
            int event_{0};
        };
    }
}
//Event事件的抽象基类，封装了文件描述符（fd）和相关的回调函数,比如如 OnRead, OnWrite等