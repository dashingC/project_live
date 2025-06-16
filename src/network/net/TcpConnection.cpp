#include <unistd.h>
#include <iostream>
#include "TcpConnection.h"
#include "network/base/Network.h"

using namespace tmms::network;
// 构造函数
TcpConnection::TcpConnection(EventLoop *loop, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : Connection(loop, sockfd, localAddr, peerAddr) // 初始化基类 Connection，传递参数
{
}
// 设置回调函数
void TcpConnection::SetCloseCallback(const CloseConnectionCallback &cb)
{
    // 将回调函数赋值给成员变量 close_cb_
    close_cb_ = cb;
}
// 设置回调函数（右值引用）
void TcpConnection::SetCloseCallback(CloseConnectionCallback &&cb)
{
    // 使用 std::move 将右值引用的回调函数移动到成员变量 close_cb_
    close_cb_ = std::move(cb);
}
// 关闭连接
void TcpConnection::OnClose()
{
    // 确保当前线程是事件循环线程
    loop_->AssertInLoopThread();

    // 如果连接尚未关闭
    if (!closed_)
    {
        // 将连接状态标记为已关闭（放到此处，避免后续TcpClient超时导致重复进入）
        // 我调用这个进来的时候，如果没有设置这个，在执行关闭还没结束的时候，可能其他线程也会调用这个
        // 此时closed_还是false,就会导致重复调用关闭逻辑，产生竞态条件
        closed_ = true;

        // 如果存在关闭回调函数
        if (close_cb_)
        {
            // 调用关闭回调，并将当前对象转换为 TcpConnection 的共享指针
            close_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
    }

    // 调用基类关闭函数，用于未执行到析构函数但需要在此处进行关闭的操作
    Event::Close();
}
// 强制关闭连接
void TcpConnection::ForceClose()
{
    // 在事件循环中调用 OnClose 函数
    /*
     * 线程安全：RunInLoop 确保 OnClose 在事件循环线程中执行，避免了跨线程调用可能导致的竞态条件和不一致状态。
     * 延迟执行：通过将 OnClose 封装在一个 lambda 表达式中，可以将其延迟到事件循环的下一个周期执行，允许当前操作完成后再处理关闭逻辑。
     * 捕获 this 指针：使用 [this] 捕获当前对象的指针，使得在 lambda 内部可以直接访问 TcpConnection 的成员函数和变量
     *
    // 本身并不立即执行关闭操作，而是将“执行关闭”这个任务，发送给管理此连接的那个唯一的I/O线程，由那个线程在未来的某个时间点去执行

    为什么需要这样做：如果另一个work线程正在调用 ForceClose() 并执行 OnClose()（比如修改closed_标志），与此同时，
    这个连接专属的I/O线程可能正在执行 OnRead() 或 OnWrite()，也在访问和修改这个连接的内部状态。就会出现数据竞争
    如何解决：任何线程调用 ForceClose()，都只是把 OnClose() 这个任务打包成一个lambda表达式，然后“投递”到目标I/O线程的邮箱里。
    I/O线程在其事件循环 (EventLoop::Loop()) 的某个安全时刻，会检查自己的邮箱，取出任务并执行。
    通过这种方式，无论有多少个外部线程想关闭连接，最终执行 OnClose() 的，永远都是那个专属的I/O线程。
    所有对 TcpConnection 状态的修改都被序列化（排队）在同一个线程里，从而完美地避免了数据竞争，也省去了使用复杂且低效的锁。

    另一方面：调用 ForceClose() 的线程，只是把任务投递出去，然后立即返回，继续执行它自己的后续代码，而不会等待 OnClose() 真正执行完毕。
    */

    /*
    RunInLoop 投递的是一个lambda函数 [](){ OnClose(); }。
    当I/O线程稍后执行这个lambda时，它不知道该调用哪个 TcpConnection 对象的 OnClose() 方法
    [this] 是C++ lambda表达式的捕获列表，它表示“捕获”当前对象的 this 指针
    当I/O线程执行这个lambda时，它会使用内部存储的 this 指针，来正确地调用那个发起请求的 TcpConnection 实例的 OnClose() 方法。
    */

    /*
    模拟一下这个跨线程的调用流程：

         场景:

             I/O线程2 是 TcpConnection实例（conn_123） 的专属线程。conn_123->loop_ 指向 I/O线程2 的 EventLoop。
             Worker线程5 是一个独立的业务处理线程。

         流程:

             1、I/O线程2 收到数据，发现是一个需要复杂计算的请求，于是它把这个请求和 conn_123 的指针打包成一个任务，
             扔给了 Worker线程5 去处理。Worker线程5 开始处理任务。在处理过程中，它发现用户提供的参数非法，决定必须立刻关闭这个连接。
             现在，代码执行权在 Worker线程5 手中。它调用：conn_123->ForceClose();
             进入 ForceClose() 函数内部（此刻，代码仍在 Worker线程5 上运行）。
             函数执行 loop_->RunInLoop(...)。这里的 loop_ 是 conn_123 的成员变量，它指向的是 I/O线程2 的 EventLoop！
             所以，RunInLoop 的效果是：
             Worker线程5 把一个 OnClose() 的任务**“隔空喊话”**，投递到了 I/O线程2 的任务队列里。
             ForceClose() 函数立即返回，Worker线程5 可以继续做别的事情。
             稍后，I/O线程2 在自己的事件循环中，从任务队列里取出了这个 OnClose() 任务，并在自己的线程里安全地执行了它。
    */
    loop_->RunInLoop([this]()
                     { OnClose(); });
}
// 读取数据
void TcpConnection::OnRead()
{
    // 检查连接是否已关闭
    if (closed_)
    {
        // 记录日志，显示对端地址已关闭连接
        NETWORK_TRACE << " host : " << peer_addr_.ToIpPort() << " had closed.";
        // 直接返回，不再处理
        return;
    }

    ExtendLife();

    // 开始一个无限循环，直到手动中断
    while (true)
    {
        // 初始化错误码
        int err = 0;
        // 从文件描述符 fd_ 中读取数据到 message_buffer_，并获取返回值和错误码
        auto ret = message_buffer_.ReadFd(fd_, &err);

        // 如果成功读取到数据
        if (ret > 0)
        {
            // 检查是否设置了消息回调
            if (message_cb_)
            {
                // 调用消息回调，传递当前对象的共享指针和消息缓冲区
                message_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()), message_buffer_);
            }
        }
        // 如果返回值为 0，表示对端关闭了连接
        else if (ret == 0)
        {
            // 调用关闭连接的函数
            OnClose();
            // 退出循环
            break;
        }
        else // 如果读取失败
        {
            // 检查错误码是否为中断、暂时不可用或非阻塞
            if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK)
            {
                // 记录读取错误的日志
                // NETWORK_ERROR << " read err : " << err;
                // 调用关闭连接的函数
                OnClose();
            }

            // 退出循环
            break;
        }
    }
}
// 设置回调函数
void TcpConnection::SetRecvMsgCallback(const MessageCallback &cb)
{
    // 将回调函数赋值给成员变量 message_cb_
    message_cb_ = cb;
}
// 设置回调函数（右值引用）
void TcpConnection::SetRecvMsgCallback(MessageCallback &&cb)
{
    // 使用 std::move 将右值引用的回调函数移动到成员变量 message_cb_
    message_cb_ = std::move(cb);
}
// 描述符出现错误
void TcpConnection::OnError(const std::string &msg)
{
    // 记录错误日志，显示对端地址和错误消息
    NETWORK_ERROR << " host : " << peer_addr_.ToIpPort() << " error msg : " << msg;
    // 调用关闭连接的函数
    OnClose();
}
// 写函数
void TcpConnection::OnWrite()
{
    // 检查连接是否已关闭
    if (closed_)
    {
        // 记录日志，显示对端地址已关闭连接
        NETWORK_TRACE << " host : " << peer_addr_.ToIpPort() << " had closed.";
        // 直接返回，不再处理
        return;
    }

    // ExtendLife();

    // 检查待写入的数据列表是否为空
    if (!io_vec_list_.empty())
    {
        // 开始一个无限循环，直到手动中断
        while (true)
        {
            // 使用 writev 函数将数据写入文件描述符 fd_，写入的起始地址，大小
            // ret 的值代表实际成功写入内核发送缓冲区的字节总数。
            auto ret = ::writev(fd_, &io_vec_list_[0], io_vec_list_.size());

            // 如果写入成功
            if (ret >= 0)
            {
                // 处理写入的字节数
                while (ret > 0)
                {
                    // 如果待发送数据块的长度大于已写入的字节数
                    if (io_vec_list_.front().iov_len > ret)
                    {
                        // 更新待发送数据块的基地址，移动已写入的字节数
                        io_vec_list_.front().iov_base = (char *)io_vec_list_.front().iov_base + ret;
                        // 更新待发送数据块的长度，减少已写入的字节数
                        io_vec_list_.front().iov_len -= ret;
                        // 退出内层循环
                        break;
                    }
                    else // 如果待发送数据块的长度小于或等于已写入的字节数
                    {
                        // 减去待发送数据块的长度
                        ret -= io_vec_list_.front().iov_len;
                        // 移除已写入的数据块
                        io_vec_list_.erase(io_vec_list_.begin());
                    }
                }

                // 如果所有数据块都已写入
                if (io_vec_list_.empty())
                {
                    // 禁用写入
                    EnableWriting(false);

                    // 检查是否设置了写入完成的回调
                    if (write_complete_cb_)
                    {
                        // 调用写入完成的回调，通知业务层 我已经将数据发送完毕
                        write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
                    }

                    // 退出函数
                    return;
                }
            }
            else // 如果写入失败
            {
                // 检查错误码是否为中断、暂时不可用或非阻塞
                if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    // 记录写入错误的日志
                    NETWORK_ERROR << " host : " << peer_addr_.ToIpPort() << " write err : " << errno;
                    // 调用关闭连接的函数
                    OnClose();
                    // 退出函数
                    return;
                }

                // 退出循环
                break;
            }
        }
    }
    else // 如果待写入的数据列表为空
    {
        // 禁用写入
        EnableWriting(false);

        // 检查是否设置了写入完成的回调
        if (write_complete_cb_)
        {
            // 调用写入完成的回调，传递当前对象的共享指针
            write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
    }
}
// 设置写完后的回调函数
void TcpConnection::SetWriteCompleteCallback(const WriteCompleteCallback &cb)
{
    // 将回调函数赋值给成员变量 write_complete_cb_
    write_complete_cb_ = cb;
}
// 设置写完后的回调函数（右值引用）
void TcpConnection::SetWriteCompleteCallback(WriteCompleteCallback &&cb)
{
    // 使用 std::move 将右值引用的回调函数移动到成员变量 write_complete_cb_
    write_complete_cb_ = std::move(cb);
}
// 发送多个、在内存中可能不连续的数据块，作为一个逻辑上的整体，按顺序发送出去（分散写）
void TcpConnection::Send(std::list<BufferNodePtr> &list)
{
    // 在事件循环中调用 SendInLoop 函数，传递数据列表
    loop_->RunInLoop([this, &list]()
                     { SendInLoop(list); });
}
// 发送一个单个的、在内存中连续存放的数据块
void TcpConnection::Send(const char *buff, size_t size)
{
    // 在事件循环中调用 SendInLoop 函数，传递缓冲区和大小
    loop_->RunInLoop([this, buff, size]()
                     { SendInLoop(buff, size); });
}
// 首先直接发送数据，如果发送不完，则将剩余数据放入 io_vec_list_ 中，等待下一次写事件触发时继续发送
void TcpConnection::SendInLoop(const char *buff, size_t size)
{
    // 检查连接是否已关闭
    if (closed_)
    {
        // 记录日志，显示对端地址已关闭连接
        NETWORK_TRACE << " host : " << peer_addr_.ToIpPort() << " had closed.";
        // 直接返回，不再处理
        return;
    }

    // 初始化一个变量 send_len 用于存储实际发送的字节数
    size_t send_len = 0;

    // 检查 io_vec_list_ 是否为空，如果为空，表示没有因为上次发送不完而积压的数据
    // 最佳情况
    if (io_vec_list_.empty())
    {
        // 调用系统的 write 函数，将数据从 buff 发送到文件描述符 fd_，并将返回的字节数存储在 send_len 中
        send_len = ::write(fd_, buff, size);

        // 检查 send_len 是否小于 0，表示写入失败
        if (send_len < 0)
        {
            // 检查错误码 errno，如果不是中断、暂时不可用或阻塞错误，记录错误日志并调用 OnClose() 关闭连接
            if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                NETWORK_ERROR << " host : " << peer_addr_.ToIpPort() << " write err : " << errno;
                OnClose();

                return;
            }

            // 如果写入失败但错误是可恢复的，将 send_len 设置为 0，把完整的buff全部交给下面的常规路径去缓冲。
            send_len = 0;
        }

        // 从待发送的字节数中减去已成功发送的字节数
        size -= send_len;

        // 检查 size 是否为 0，说明所有数据一次性发送成功！ 
        // 没有发送完成也没有关系，记下了成功发送的字节数send_len，
        // 剩下的部分（size也已更新）就交给下面的常规路径去处理。
        if (size == 0)
        {
            // 如果 write_complete_cb_（写完成的回调函数）存在，则执行以下操作
            if (write_complete_cb_)
            {
                // 使用 std::dynamic_pointer_cast 将当前对象（通过 shared_from_this() 获取）转换为 TcpConnection 类型的智能指针，并调用回调函数
                write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
            }

            // 结束当前函数的执行
            return;
        }
    }

    // 如果还有未发送的数据
    if (size > 0)
    {
        // 创建一个 iovec 结构体
        struct iovec vec;
        // 设置 iov_base 为 buff 的偏移地址，指向未发送的数据
        //这里为什么要加上 send_len 呢？因为 send_len 是已经发送的字节数，但是没有发送完
        vec.iov_base = (void *)(buff + send_len);
        // 设置 iov_len 为剩余的待发送字节数
        vec.iov_len = size;

        // 将 iovec 结构体添加到 io_vec_list_ 中，准备后续发送
        io_vec_list_.push_back(vec);

        // 调用 EnableWriting 函数，启用写入操作
        EnableWriting(true);
    }
}
// 发送数据，处理多个 BufferNodePtr 列表中的数据
void TcpConnection::SendInLoop(std::list<BufferNodePtr> &list)
{
    // 检查连接是否已关闭。如果是，记录日志并返回
    if (closed_)
    {
        NETWORK_TRACE << " host : " << peer_addr_.ToIpPort() << " had closed.";
        return;
    }

    // 遍历传入的 BufferNodePtr 列表
    for (auto &l : list)
    {
        // 定义一个 iovec 结构体
        struct iovec vec;
        // 设置 iov_base 为当前 BufferNodePtr 的地址
        vec.iov_base = (void *)l->addr;
        // 设置 iov_len 为当前 BufferNodePtr 的大小
        vec.iov_len = l->size;

        // 将 iovec 结构体添加到 io_vec_list_ 中
        io_vec_list_.push_back(vec);
    }

    // 如果 io_vec_list_ 不为空，调用 EnableWriting(true); 启用写入操作
    if (!io_vec_list_.empty())
    {
        EnableWriting(true);
    }
}
// 超时关闭连接
void TcpConnection::OnTimeout()
{
    NETWORK_ERROR << " host : " << peer_addr_.ToIpPort() << " timeout and close it.";
    std::cout << "host : " << peer_addr_.ToIpPort() << " timeout and close it." << std::endl;
    OnClose();
}
// 设置定时
void TcpConnection::EnableCheckIdleTimeout(int32_t max_time)
{
    //补充 std::make_shared<TimeoutEntry>(conn_sptr),这里的 conn_sptr 类型是 std::shared_ptr<TcpConnection>
    // std::make_shared<TimeoutEntry>(conn_sptr)会做两件事情：
    // 1、它会去调用 TimeoutEntry 类的构造函数
    // 2、它会把我们准备好的参数 conn_sptr 传递给这个构造函数
    //最后我们得到了一个智能指针tp，这个tp的类型是 std::shared_ptr<TimeoutEntry>，
    //并且它指向了一个 TimeoutEntry 对象.
    auto tp = std::make_shared<TimeoutEntry>(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
    max_idle_time_ = max_time;
    // 把一个强指针赋值给 timeout_entry_这个弱指针
    timeout_entry_ = tp;
    // 将超时条目插入到事件循环的时间轮中
    loop_->InsertEntry(max_time, tp);
}
// 设置定时回调函数
/*
SetTimeoutCallback 的逻辑是：
        上层应用说：“请5秒后执行 my_custom_task”。
        SetTimeoutCallback 就对 EventLoop 说：“EventLoop，请你用你的 RunAfter 能力，
                                                帮我把这个 my_custom_task 注册到你的时间轮里，让它5秒后被执行。”
*/
void TcpConnection::SetTimeoutCallback(int timeout, const TimeoutCallback &cb)
{
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
    loop_->RunAfter(timeout, [&cp, &cb]()
                    { cb(cp); });
}
// 设置定时回调函数（右值引用）
void TcpConnection::SetTimeoutCallback(int timeout, TimeoutCallback &&cb)
{
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
    loop_->RunAfter(timeout, [&cp, cb]()
                    { cb(cp); });
}
//延长时间
void TcpConnection::ExtendLife()
{
    // 第一步：找到我之前设置的那个“闹钟”
    auto tp = timeout_entry_.lock();
    // 如果还在时间轮上，重新设置定时
    if (tp)
    {
        loop_->InsertEntry(max_idle_time_, tp);
    }
}

TcpConnection::~TcpConnection()
{
    // 处理连接关闭的逻辑
    OnClose();
}