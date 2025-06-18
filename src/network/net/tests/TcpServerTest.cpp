#include <iostream>
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpServer.h"
// #include "network/TestContext.h"

using namespace tmms::network;

// 创建事件循环线程对象
EventLoopThread eventloop_thread;
// 创建一个线程对象
std::thread th;

// 定义TestContext智能指针
// using TestContextPtr = std::shared_ptr<TestContext>;

// 定义HTTP响应字符串
const char *http_response = "HTTP/1.0 200 OK\r\nServer: qcy\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

int main(int argc, const char **agrv)
{
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    // 如果事件循环有效
    if (loop)
    {
        // 创建一个InetAddress对象，指定监听地址和端口
        InetAddress listen("192.168.222.100:34444");
        // 创建TcpServer对象
        TcpServer server(loop, listen);

        // 设置消息回调函数
        server.SetMessageCallback([](const TcpConnectionPtr &con, MsgBuffer &buff){
            // TestContextPtr context = con->GetContext<TestContext>(kNormalContext);

            // context->ParseMessage(buff);

            // 输出接收到的消息
            std::cout << "host: " << con->PeerAddr().ToIpPort() << " msg: " << buff.Peek() << std::endl;
            // 清空消息缓冲区
            buff.RetrieveAll();
            // 发送HTTP响应
            con->Send(http_response, strlen(http_response));
        });

        // 设置新连接回调函数
        // server.SetNewConnectionCallback([&loop](const TcpConnectionPtr &con){
        //         TestContextPtr context = std::make_shared<TestContext>(con);

        //         context->SetTestMessageCallback([](const TcpConnectionPtr &con, const std::string &msg){
        //             std::cout << "message: " << msg << std::endl;
        //         });
                
        //         con->SetContext(kNormalContext,context);

        //         // 设置写完成回调函数
        //         con->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &con){
        //         // 输出写完成信息
        //         std::cout << "write complete host: " << con->PeerAddr().ToIpPort() << std::endl;
        //         // 强制关闭连接
        //         // con->ForceClose();
        //     });
        // });
        server.SetNewConnectionCallback([&loop](const TcpConnectionPtr &con){
            con->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &con){
                // 输出写完成信息
                std::cout << "write complete host: " << con->PeerAddr().ToIpPort() << std::endl;
                con->ForceClose();
            });

        });
        // 启动TcpServer
        server.Start();

        // 无限循环，保持程序运行
        while (1)
        {
            // 每秒休眠一次
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    

    return 0;
}