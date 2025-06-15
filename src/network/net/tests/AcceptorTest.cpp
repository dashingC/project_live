#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"

#include <iostream>

using namespace tmms::network;

// 首先定义一个事件循环
EventLoopThread eventloop_thread;
// 定义了一个名为 th 的线程对象，但它没有绑定任何线程函数，因此也没有启动线程
std::thread th;

int main(int argc, char **argv)
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress server("192.168.222.100:34444");
        std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, server);
        acceptor->SetAcceptCallback([](int fd, const InetAddress addr)
                                    { std::cout << "host:" << addr.ToIpPort() << std::endl; });
        acceptor->Start();
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return 0;
}