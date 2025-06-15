#include "Acceptor.h"
#include "network/base/Network.h"
using namespace tmms::network;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &addr)
    : Event(loop), addr_(addr)
{
}
Acceptor::~Acceptor()
{
    Stop();
    if (socket_opt_)
    {
        delete socket_opt_;
        socket_opt_ = nullptr;
    }
}

void Acceptor::SetAcceptCallback(const AcceptCallback &cb)
{
    accept_cb_ = cb;
}

void Acceptor::SetAcceptCallback(AcceptCallback &&cb)
{
    accept_cb_ = std::move(cb);
}

void Acceptor::Open()
{
    // 1. 清理旧资源（如果存在）
    if (fd_ > 0)
    {
        ::close(fd_);
        fd_ = -1;
    }
    // 2. 创建一个新的、非阻塞的TCP socket
    if (addr_.IsIpV6())
    {
        fd_ = SocketOpt::CreateNonblockingTcpSocket(AF_INET6);
    }
    else
    {
        fd_ = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
    }
     // 3. 检查socket是否创建成功
    if (fd_ < 0)
    {
        NETWORK_ERROR << "socket failed.errno:" << errno;
        exit(-1);
    }

    // 4. 再次清理旧资源（SocketOpt对象）
    if (socket_opt_)
    {
        delete socket_opt_;
        socket_opt_ = nullptr;
    }
    
    // 5. 将Acceptor自身注册到EventLoop中
    // AddEvent需要一个EventPtr类型的指针
    // shared_from_this()创建了一个指向自己的智能指针
    // 请求 EventLoop 开始监听自己身上的事件
    loop_->AddEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));
    
    // 6. 配置、绑定并监听socket
    socket_opt_ = new SocketOpt(fd_);
    socket_opt_->SetReuseAddr(true);
    socket_opt_->SetReusePort(true);
    socket_opt_->BindAddress(addr_);
    socket_opt_->Listen();
}

void Acceptor::Start()
{
    // 将Acceptor的启动任务(Open)提交到其所属的事件循环中执行
    loop_->RunInLoop([this]()
                     { Open(); });
}

void Acceptor::Stop()
{
    loop_->DelEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));
}

void Acceptor::OnRead()
{
    if (!socket_opt_)
    {
        return;
    }
    while (true)
    {
        InetAddress addr;
        auto sock = socket_opt_->Accept(&addr);
        if (sock > 0)
        {
            if (accept_cb_)
            {
                accept_cb_(sock, addr);
            }
        }
        else
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                NETWORK_ERROR << "accept failed.errno:" << errno;
                OnClose();
            }
            break;
        }
    }
}

void Acceptor::OnError(const std::string &msg)
{
    NETWORK_ERROR << "accept failed.errno:" << msg;
    OnClose();
}

void Acceptor::OnClose()
{
    Stop();
    Open();
}