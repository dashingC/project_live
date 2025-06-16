#include "Connection.h"

using namespace tmms::network;

// Connection 类的构造函数，接受一个事件循环指针、文件描述符和两个 InetAddress 类型的参数，分别表示本地地址和远端地址
Connection::Connection(EventLoop *loop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : Event(loop, fd) // 调用基类 Event 的构造函数，接受一个事件循环指针和文件描述符进行初始化
      ,
      local_addr_(localAddr) // 初始化成员变量 local_addr_ 为本地地址
      ,
      peer_addr_(peerAddr) // 初始化成员变量 peer_addr_ 为远端地址
{
}

// 设置本地地址
void Connection::SetLocalAddr(const InetAddress &local)
{
    local_addr_ = local;
}

// 设置远端地址
void Connection::SetPeerAddr(const InetAddress &peer)
{
    peer_addr_ = peer;
}

// 获取本地地址的常量引用
const InetAddress &Connection::LocalAddr() const
{
    return local_addr_;
}

// 获取远端地址的常量引用
const InetAddress &Connection::PeerAddr() const
{
    return peer_addr_;
}

// 设置特定类型的上下文，接受一个类型和一个共享指针作为参数，并将它们存储在 contexts_ 映射中
void Connection::SetContext(int type, const std::shared_ptr<void> &context)
{
    contexts_[type] = context;
}

// 重载成员函数，使用右值引用接受 context ，并将 context 移动到 contexts_ 映射中，而不是复制
void Connection::SetContext(int type, std::shared_ptr<void> &&context)
{
    contexts_[type] = std::move(context);
}

// 清除特定类型的上下文，接受一个类型作为参数
void Connection::ClearContext(int type)
{
    contexts_[type].reset();
}

// 清除所有上下文
void Connection::ClearContext()
{
    contexts_.clear();
}

// 设置活动回调函数，接受一个 ActiveCallback 类型的参数
void Connection::SetActiveCallback(const ActiveCallback &cb)
{
    active_cb_ = cb;
}

// 重载成员函数，使用右值引用接受 cb ，并将 cb 移动到 active_cb_
void Connection::SetActiveCallback(ActiveCallback &&cb)
{
    active_cb_ = std::move(cb);
}

// 激活连接
void Connection::Active()
{
    // 检查 active_ 标志是否为 false
    if (!active_.load())
    {
        // 如果是，在事件循环中运行一个 lambda 函数
        loop_->RunInLoop([this]()
                         {
            // 设置 active_ 标志为 true
            active_.store(true);
            
            // 如果存在激活回调函数，调用
            if (active_cb_)
            {
                // 传递 Connection 对象的 shared_ptr
                // 补充：当一个对象（这里是 Connection 对象）的生命周期被 std::shared_ptr 管理时，
                // shared_from_this() 可以在该对象内部安全地获取一个指向自身的智能指针，类型：std::shared_ptr<Event>
                // 这个新的 shared_ptr 会和所有其他管理这个对象的 shared_ptr 共享所有权（即共享同一个引用计数）
                // 在一个 Connection 对象内部调用 shared_from_this()，你调用的实际上是
                // 从 Event 继承来的、被配置为返回 std::shared_ptr<Event> 的那个函数。
                // 这里是一个 Connection 对象，所以再安全地把它向下转型为我们真正需要的派生类指针 (std::shared_ptr<Connection>)
                // 而active_cb_接受一个类型为ConnectionPtr&的参数
                active_cb_(std::dynamic_pointer_cast<Connection>(shared_from_this()));
            } });
    }
}

// 停用连接
void Connection::Deactive()
{
    // 将active_标志设置为false
    active_.store(false);
}