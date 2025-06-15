#pragma once
/*
    网络编程经常要用的 IP 和端口，传递两个值比较麻烦
    IP 和端口经常需要进行转换成其他形式
    有时候需要对地址进行分类检测
    InetAddress 类主要方便存储 IP 和端口信息，提供地址相关的操作
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <bits/socket.h>

namespace tmms {
    namespace network {
        class InetAddress {
        public:
            // 通过独立的 IP 字符串和 16位无符号整数端口号来构造
            InetAddress(const std::string &ip, uint16_t port, bool bv6=false);
            // 通过一个单一的 host 字符串（格式为 "ip:port"）来构造
            InetAddress(const std::string &host, bool is_v6=false);
            InetAddress() = default;
            ~InetAddress() = default; 

            // 赋值函数
            void SetHost(const std::string &host);
            void SetAddr(const std::string &addr);
            void SetPort(uint16_t port);
            void SetIsIPV6(bool is_v6);

            // 取值函数
            const std::string &IP() const;                          // 返回字符串形式的 IP 地址
            uint32_t IPv4() const;                                  // 返回 uint32_t 形式的 IPv4 地址（主机字节序）
            std::string ToIpPort() const;                           // 返回 "ip:port" 格式的完整地址字符串
            uint16_t Port() const;                                  // 返回 uint16_t 形式的端口号
            
            void GetSockAddr(struct sockaddr *addr) const;          // 将 InetAddress 转换为 sockaddr 结构体

            // 测试函数
            bool IsIpV6() const;
            bool IsWanIp() const;
            bool IsLanIp() const;
            bool IsLoopbackIp() const;

            static void GetIpAndPort(const std::string &host, std::string &ip, std::string &port);

        private:
            // 私有取值
            uint32_t IPv4(const char *ip) const;
            // 私有成员变量
            std::string addr_;
            std::string port_;
            bool is_ipv6_{false};
        };
    }
}