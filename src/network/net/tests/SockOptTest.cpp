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

    std::cout << "connect ret: " << ret << "errorn:" << errno << std::endl
              << "local:" << opt.GetLocalAddr()->ToIpPort() << std::endl
              << "PEER:" << opt.GetPeerAddr()->ToIpPort() << std::endl
              << std::endl;
}

void testServer()
{
    int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);

    if (sock < 0)
    {
        std::cerr << "create socket failed" << std::endl;
        return;
    }
    InetAddress sever("0.0.0.0:3444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    opt.BindAddress(sever);
    opt.Listen();
    InetAddress addr;
    auto ret = opt.Accept(&addr);

    std::cout << "connect ret: " << ret << " errorn:" << errno << std::endl
              << "addr: " << addr.ToIpPort() << std::endl
              << std::endl;
}

int main(int argc, const char **argv)
{
    testClient();
    // testServer();
    return 0;
}