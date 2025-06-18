#include <iostream>
#include "network/DnsService.h"

using namespace tmms::network;


int main(int argc, const char **argv)
{
    std::string myadd = "www.qq.com";
    // 创建存储地址信息的向量
    std::vector<InetAddressPtr> list;

    // 添加域名
    sDnsService->AddHost(myadd);

    // 启动服务
    sDnsService->Start();

    // 等待2s
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 获取结果
    list = sDnsService->GetHostAddress(myadd);

    // 获取主机信息
    // DnsService::GetHostInfo("www.baidu.com", list);

    // 遍历地址列表
    for (auto &i : list)
    {
        // 输出 IP 地址
        std::cout << "ip: " << i->IP() << std::endl;
    }

    return 0;
}