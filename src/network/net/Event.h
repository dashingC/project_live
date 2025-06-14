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

        protected:
            EventLoop * loop_{nullptr};
            int fd_{-1};
            int event_{0};
        };
    }
}
//Event事件的抽象基类，封装了文件描述符（fd）和相关的回调函数,比如如 OnRead, OnWrite等