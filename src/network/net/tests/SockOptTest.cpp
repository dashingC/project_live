#include "network/base/SocketOpt.h"
#include "network/base/InetAddress.h"
#include <iostream>

using namespace tmms::network;

void testClient()
{
    int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);

    if (sock < 0)
    {
        std::cerr << "create socket failed" << std::endl;
        return;
    }
    InetAddress sever("192.168.222.100:3444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    auto ret = opt.Connect(sever);

    std::cout << "connect ret: " << ret << "  errorn:" << errno << std::endl
              << "local:" << opt.GetLocalAddr()->ToIpPort() << std::endl
              << "PEER:" << opt.GetPeerAddr()->ToIpPort() << std::endl
              << std::endl;
}

void testServer()
{
    // 创建一个Socket
    int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);

    if (sock < 0)
    {
        std::cerr << "create socket failed" << std::endl;
        return;
    }
    InetAddress sever("0.0.0.0:3444");

    // 相当于把sock传给opt,直接调用操作就可以了。不再把 socket 看作一个简单的整数，
    // 而是把它升级成一个功能强大、有自身行为、能自我管理的“对象”（SocketOpt 的实例）
    SocketOpt opt(sock);

    opt.SetNonBlocking(false);
    opt.BindAddress(sever);
    opt.Listen();
    InetAddress addr;

    // 主线程会阻塞在这儿，等待客户端的连接
    //  ret 代表的是一个全新的、专门用于和刚刚连接进来的这个客户端进行通信的 socket
    auto ret = opt.Accept(&addr);

    // 
    std::cout << "connect ret: " << ret << " errorn:" << errno << std::endl
              << "addr: " << addr.ToIpPort() << std::endl
              << std::endl;
}

int main(int argc, const char **argv)
{
    // testClient();
    testServer();
    return 0;
}