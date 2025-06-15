
#include "PipeEvent.h"
#include "network/base/Network.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <errno.h>
using namespace tmms::network;

PipeEvent::PipeEvent(EventLoop *loop) : Event(loop)
{
    int fd[2] = {
        0,
    };
     // 创建一个管道，并设置为非阻塞模式
    auto ret = ::pipe2(fd, O_NONBLOCK);
    if (ret < 0)
    {
        NETWORK_ERROR << "pipe2 open failed.";
        exit(-1);
    }
    fd_ = fd[0];//读端， 交给基类Event管理，用于被EventLoop监听
    write_fd_ = fd[1];// 写端，由PipeEvent自己持有，用于给其他线程写入
}
PipeEvent::~PipeEvent()
{
    if (write_fd_ > 0)
    {
        ::close(write_fd_);
        write_fd_ = -1;
    }
}

void PipeEvent::OnRead()
{
    int64_t tmp = 0;
    auto ret = ::read(fd_, &tmp, sizeof(tmp));
    if (ret < 0)
    {
        NETWORK_ERROR << "pipe read error. ret:" << errno;
        return;
    }
    std::cout << "管道读取到的值为：" << tmp << std::endl;
}

void PipeEvent::OnClose()
{
    if (write_fd_ > 0)
    {
        ::close(write_fd_);
        write_fd_ = -1;
    }
}

void PipeEvent::OnError(const std::string &msg)
{
    std::cout << "error:" << msg << std::endl;
}

void PipeEvent::Write(const char *data, size_t len)
{
    ::write(write_fd_, data, len);
}
