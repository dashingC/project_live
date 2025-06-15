#include "InetAddress.h"
#include "Network.h"
#include <cstring>

using namespace tmms::network;


// 获取IP和端口
void InetAddress::GetIpAndPort(const std::string &host, std::string &ip, std::string &port)
{
    auto pos = host.find_first_of(':', 0); // 查找第一个冒号
    if (pos != std::string::npos)
    {
        ip = host.substr(0, pos);          // 冒号前是IP
        port = host.substr(pos + 1);       // 冒号后是端口
    }
    else
    {
        ip = host;
    }
}
// 构造函数
InetAddress::InetAddress(const std::string &ip, uint16_t port, bool bv6)
    : addr_(ip), port_(std::to_string(port)), is_ipv6_(bv6)
{
}
InetAddress::InetAddress(const std::string &host, bool is_v6)
{
    GetIpAndPort(host, addr_, port_);
    is_ipv6_ = is_v6;
}

// 设置主机地址
void InetAddress::SetHost(const std::string &host)
{
    GetIpAndPort(host, addr_, port_);
}
// 设置IP
void InetAddress::SetAddr(const std::string &addr)
{
    addr_ = addr;
}
// 设置端口
void InetAddress::SetPort(uint16_t port)
{
    port_ = std::to_string(port);
}
// 设置是否为IPv6地址
void InetAddress::SetIsIPV6(bool is_v6)
{
    is_ipv6_ = is_v6;
}

// 返回IP地址的字符串形式
const std::string &InetAddress::IP() const
{
    return addr_;
}

// 给类外部使用的接口，唯一工作就是调用那个私有的、带有参数的 IPv4 版本
uint32_t InetAddress::IPv4() const
{
    return IPv4(addr_.c_str());
}

// 返回主机字节序的 IPv4 地址
uint32_t InetAddress::IPv4(const char *ip) const
{
    // 创建并初始化 socket 地址结构体
    struct sockaddr_in addr_in;
    memset(&addr_in, 0x00, sizeof(sockaddr_in));
    // 设置地址族
    addr_in.sin_family = AF_INET;
    // 设置端口，这个函数的目标仅仅是转换 IP 地址本身，并不关心端口
    addr_in.sin_port = 0;
    // inet_pton：将一个人类可读的 IP 地址字符串，转换成计算机网络系统所使用的二进制格式
    // 转换后的IP地址存放在 addr_in.sin_addr 中
    if (::inet_pton(AF_INET, ip, &addr_in.sin_addr) < 0)
    {
        NETWORK_ERROR << "ipv4 ip :" << ip << " is error";
        return 0;
    }
    // 网络字节序到主机字节序的转换
    return ntohl(addr_in.sin_addr.s_addr);
}

// 返回IP地址和端口的字符串形式
std::string InetAddress::ToIpPort() const
{
    std::stringstream ss;
    ss << addr_ << ":" << port_;
    return ss.str();
}

// 返回端口号
uint16_t InetAddress::Port() const
{
    return atoi(port_.c_str());
}

//将 C++ 的 InetAddress 对象内部存储的地址信息，转换并打包成一个 C 语言底层网络库能够识别和使用的标准地址结构体  
void InetAddress::GetSockAddr(struct sockaddr *saddr) const
{
    if (is_ipv6_)
    {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)saddr;
        memset(addr_in6, 0x00, sizeof(struct sockaddr_in6));

        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(Port());
        if (::inet_pton(AF_INET6, addr_.c_str(), &addr_in6->sin6_addr) < 0)
        {
        }
        return;
    }

    // 将通用指针转换为专用指针
    struct sockaddr_in *addr_in = (struct sockaddr_in *)saddr;
    memset(addr_in, 0x00, sizeof(struct sockaddr_in));

    // 设置地址族：表明这是IPv4
    addr_in->sin_family = AF_INET;
    // 设置端口号：将端口转换为网络字节序
    addr_in->sin_port = htons(Port());
    if (::inet_pton(AF_INET, addr_.c_str(), &addr_in->sin_addr) < 0)
    {
    }
}

// 判断是否为IPV6地址
bool InetAddress::IsIpV6() const
{
    return is_ipv6_;
}

// 判断是否是广域网IP
bool InetAddress::IsWanIp() const
{
    uint32_t a_start = IPv4("10.0.0.0");
    uint32_t a_end = IPv4("10.255.255.255");
    uint32_t b_start = IPv4("172.16.0.0");
    uint32_t b_end = IPv4("172.31.255.255");
    uint32_t c_start = IPv4("192.168.0.0");
    uint32_t c_end = IPv4("192.168.255.255");

    uint32_t ip = IPv4();
    bool is_a = (ip >= a_start && ip <= a_end);
    bool is_b = (ip >= b_start && ip <= b_end);
    bool is_c = (ip >= c_start && ip <= c_end);

    return !is_a && !is_b && !is_c && ip != INADDR_LOOPBACK;
}

// 判断是否是局域网IP
bool InetAddress::IsLanIp() const
{
    uint32_t a_start = IPv4("10.0.0.0");
    uint32_t a_end = IPv4("10.255.255.255");
    uint32_t b_start = IPv4("172.16.0.0");
    uint32_t b_end = IPv4("172.31.255.255");
    uint32_t c_start = IPv4("192.168.0.0");
    uint32_t c_end = IPv4("192.168.255.255");

    uint32_t ip = IPv4();
    bool is_a = ip >= a_start && ip <= a_end;
    bool is_b = ip >= b_start && ip <= b_end;
    bool is_c = ip >= c_start && ip <= c_end;

    return is_a || is_b || is_c;
}

// 判断是否是回环IP
bool InetAddress::IsLoopbackIp() const
{
    return addr_ == "127.0.0.1";
}