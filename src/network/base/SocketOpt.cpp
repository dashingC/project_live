#include "SocketOpt.h"
#include "Network.h"

using namespace tmms::network;

SocketOpt::SocketOpt(int sock, bool v6) : sock_(sock), is_v6_(v6)
{
}

// 创建TCP接口
int SocketOpt::CreateNonblockingTcpSocket(int family)
{
    // 创建一个非阻塞的TCP套接字，在调用exec系列函数时，自动关闭这个socket
    int sock = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock < 0)
    {
        NETWORK_ERROR << "create tcp socket failed";
        return -1;
    }
    return sock;
}
// 创建UDP接口
int SocketOpt::CreateNonblockingUdpSocket(int family)
{
    int sock = ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    if (sock < 0)
    {
        NETWORK_ERROR << "create udp socket failed";
        return -1;
    }
    return sock;
}

//  服务端将 socke绑定到一个本地 IP 地址和端口（localaddr）上
int SocketOpt::BindAddress(const InetAddress &localaddr)
{
    if (localaddr.IsIpV6())
    {
        struct sockaddr_in6 addr;
        localaddr.GetSockAddr((struct sockaddr *)&addr);
        socklen_t size = sizeof(struct sockaddr_in6);
        return ::bind(sock_, (struct sockaddr *)&addr, size);
    }
    else
    {
        struct sockaddr_in addr;
        localaddr.GetSockAddr((struct sockaddr *)&addr);
        socklen_t size = sizeof(struct sockaddr_in);
        return ::bind(sock_, (struct sockaddr *)&addr, size);
    }
}

// 服务端监听
int SocketOpt::Listen()
{ 
    // SOMAXCONN是等待队列的长度
    return ::listen(sock_, SOMAXCONN);
}

// 服务端接受连接
int SocketOpt::Accept(InetAddress *peeraddr)
{
    // sockaddr_in6 (用于 IPv6) 比 sockaddr_in (用于 IPv4) 所占的内存空间更大。
    // 通过使用这个最大的可能性容器，可以同时兼容处理 IPv4 和 IPv6 的连接，而无需写两套不同的 accept 调用。
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    // 第一个参数 sock_: 监听套接字，正在监听客户端连接
    // 第二个参数 (struct sockaddr *)&addr: 这是指向我们准备好的地址容器的指针强，制转换为通用的 struct sockaddr * 类型
    // 第三个参数 &len: 定义的长度变量的指针
    // SOCK_CLOEXEC: 如果当前进程通过 fork() 创建了子进程，然后子进程调用 exec() 执行新程序时，这个新创建的套接字文件描述符会被自动关闭。
    int sock = ::accept4(sock_, (struct sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (sock > 0)
    {
        // 如果是IPV4
        if (addr.sin6_family == AF_INET)
        {
            char ip[16] = {0};
            struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
            // 将二进制的网络格式 IP 地址，转换回人类可读的字符串格式
            ::inet_ntop(AF_INET, &(saddr->sin_addr.s_addr), ip, sizeof(ip)); 
            peeraddr->SetAddr(ip);
            // saddr->sin_port: 从 sockaddr_in 结构体中取出端口号。这个值是内核给我们的，它是网络字节序的！
            // ntohs(saddr->sin_port):网络字节序转换回当前计算机可以正常计算和显示的主机字节序
            peeraddr->SetPort(ntohs(saddr->sin_port));
        }
        else if (addr.sin6_family == AF_INET6)
        {
            char ip[INET6_ADDRSTRLEN] = {
                0,
            };
            ::inet_ntop(AF_INET6, &(addr.sin6_addr), ip, sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(addr.sin6_port));
            peeraddr->SetIsIPV6(true);
        }
    }
    return sock;
}

// 客户端
int SocketOpt::Connect(const InetAddress &addr)
{
    struct sockaddr_in6 addr_in;
    addr.GetSockAddr((struct sockaddr *)&addr_in);
    return ::connect(sock_, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in6));
}

// 返回本机所绑定的 IP 地址和端口号
InetAddressPtr SocketOpt::GetLocalAddr()
{
    struct sockaddr_in6 addr_in;
    socklen_t len = sizeof(struct sockaddr_in6);
    // 查到的结果会存储在 addr_in 中
    ::getsockname(sock_, (struct sockaddr *)&addr_in, &len);
    InetAddressPtr peeraddr = std::make_shared<InetAddress>();
    // 根据 addr_in 的地址族（IPv4 或 IPv6）来设置 InetAddress 对象的 IP 地址和端口号
    if (addr_in.sin6_family == AF_INET)
    {
        char ip[16] = {0};
        struct sockaddr_in *saddr = (struct sockaddr_in *)&addr_in;
        ::inet_ntop(AF_INET, &(saddr->sin_addr.s_addr), ip, sizeof(ip));
        peeraddr->SetAddr(ip);
        peeraddr->SetPort(ntohs(saddr->sin_port));
    }
    else if (addr_in.sin6_family == AF_INET6)
    {
        char ip[INET6_ADDRSTRLEN] = {
            0,
        };
        ::inet_ntop(AF_INET6, &(addr_in.sin6_addr), ip, sizeof(ip));
        peeraddr->SetAddr(ip);
        peeraddr->SetPort(ntohs(addr_in.sin6_port));
        peeraddr->SetIsIPV6(true);
    }
    return peeraddr;
}

// 返回对端（Peer）的 IP 地址和端口号
InetAddressPtr SocketOpt::GetPeerAddr()
{
    struct sockaddr_in6 addr_in;
    socklen_t len = sizeof(struct sockaddr_in6);
    ::getpeername(sock_, (struct sockaddr *)&addr_in, &len);
    InetAddressPtr peeraddr = std::make_shared<InetAddress>();
    if (addr_in.sin6_family == AF_INET)
    {
        char ip[16] = {0};
        struct sockaddr_in *saddr = (struct sockaddr_in *)&addr_in;
        ::inet_ntop(AF_INET, &(saddr->sin_addr.s_addr), ip, sizeof(ip));
        peeraddr->SetAddr(ip);
        peeraddr->SetPort(ntohs(saddr->sin_port));
    }
    else if (addr_in.sin6_family == AF_INET6)
    {
        char ip[INET6_ADDRSTRLEN] = {
            0,
        };
        ::inet_ntop(AF_INET6, &(addr_in.sin6_addr), ip, sizeof(ip));
        peeraddr->SetAddr(ip);
        peeraddr->SetPort(ntohs(addr_in.sin6_port));
        peeraddr->SetIsIPV6(true);
    }
    return peeraddr;
}



// 补充： 当你发送很小的数据包时（比如只有一个字节），TCP 不会立即发送。
// 它会“稍等片刻”，希望能“攒一波”更多的数据，然后将它们合并成一个较大的数据包再一起发送出去。
// 这样做可以减少网络中“小包”的数量，降低 TCP/IP 头部的开销，提高网络总吞吐量。
// 但是引入了延迟 (Latency)。对于那些对实时性要求极高的应用（比如在线游戏、金融高频交易），
// 零点几秒的延迟都是不可接受的。
// 开启 TCP_NODELAY (即 SetTcpNoDelay(true)) 是以牺牲一些网络效率为代价，来换取最低的数据传输延迟。
void SocketOpt::SetTcpNoDelay(bool on)
{
    int optvalue = on ? 1 : 0;
    ::setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, &optvalue, sizeof(optvalue));
}

// 当一个 TCP 连接正常关闭时，发起关闭的一方会进入一个名为 TIME_WAIT 的状态，并持续一到两分钟。
// 在此期间，这个连接所使用的 IP:端口 组合会被操作系统“保留”，不允许任何新连接使用。
// 这是为了确保网络中可能存在的、延迟到达的旧数据包不会干扰到新建立的连接。
// 这在服务器开发中非常恼人。当你停止服务器再立即重启时，服务器尝试绑定到同一个端口，
// 但操作系统会因为该端口处于 TIME_WAIT 状态而拒绝绑定，。
// 允许绑定到一个正处于 TIME_WAIT 状态的地址上
void SocketOpt::SetReuseAddr(bool on)
{
    int optvalue = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue));
}

// 允许多个独立的进程或线程，绑定到完全相同的 IP 和端口上。
void SocketOpt::SetReusePort(bool on)
{
    int optvalue = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_REUSEPORT, &optvalue, sizeof(optvalue));
}

// 客户端和服务器建立了连接后，如果客户端突然断网、死机或被拔掉网线，服务器端是无法立即感知的。
// 在服务器看来，这个连接依然是“活跃”的，它会一直占用着服务器的资源，形成“僵尸连接”。
// 当这个选项开启后，如果一个连接在很长一段时间内（默认通常是 2 小时）没有任何数据往来，TCP 协议栈会自动向对端发送一个“保活探测包”。
// 如果收到响应，说明连接正常，计时器重置。
// 如果连续发送多个探测包都未收到响应，内核就会认为这个连接已经“死亡”，并自动关闭它，释放相关资源。
void SocketOpt::SetKeepAlive(bool on)
{
    int optvalue = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_KEEPALIVE, &optvalue, sizeof(optvalue));
}

void SocketOpt::SetNonBlocking(bool on)
{
    int flag = ::fcntl(sock_, F_GETFL, 0);
    if (on)
    {
        flag |= O_NONBLOCK;
    }
    else
    {
        flag &= ~O_NONBLOCK;
    }

    ::fcntl(sock_, F_SETFL, flag);
}