#include "base/Logger.h"
#include "base/LogStream.h"
#include "FileLog.h"
#include "FileMgr.h"
#include "TTime.h"
#include "TaskMgr.h"
#include <thread>

using namespace tmms::base;

std::thread t;

void TestLog()
{
    t = std::thread([]()
                    {
                        while (true)
                        {
                            // LOG_TRACE << "This is a trace log message.  now:" << TTime::NowMS();
                            LOG_DEBUG << "This is a debug log message.  now:" << TTime::NowMS();
                            LOG_INFO << "This is an info log message.   now:" << TTime::NowMS();
                            // LOG_WARN << "This is a warning log message. now:" << TTime::NowMS();
                            // LOG_ERROR << "This is an error log message. now:" << TTime::NowMS();
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        } });
}
int main()
{
    FileLogPtr log = sFileMgr->GetFileLog("test.log");//创建一个文件日志对象，FileLog 是日志的“执行者”：它只关心怎么把字符串写进物理文件里
    log->SetRotate(kRotateMinute);

    tmms::base::g_logger = new Logger(log);//Logger 是日志的“管理者”：它关心的是日志的规则（比如什么级别的日志该写），并将任务派发给“执行者”

    //Logger 类的职责：制定日志策略：它的核心功能是根据设置的日志级别 level_ 来判断一条消息是否应该被记录。
    //不关心“如何写”：它自己不执行任何文件 I/O 操作。当它决定要记录一条消息时，它会把自己持有的 FileLog 对象（就是那支“钢笔”）拿出来，
    //然后调用 log_->WriteLog(msg) 把任务交给 FileLog 去完成。

    tmms::base::g_logger->SetLogLevel(kTrace);

    //创建一个名为 task4 的智能任务对象，这个任务被设定为每隔 1000 毫秒执行一次。
    //每次执行时，它会调用 sFileMgr->OnCheck() 来检查日志是否需要切分，然后立即将自己重置，准备在 1000 毫秒后再次执行。
    TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task)
                                           {
                                            sFileMgr->OnCheck();
                                            task->Restart();
                                            }, 
                                            1000);
    TestLog();
    while(true)
    {
        STaskMgr->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}

// 整个日志系统的流程：

// 用户：通过 LOG_DEBUG << "..." 宏轻松地写日志。
// LogStream (秘书)：被临时创建，负责将用户的零散信息（字符串、数字）与上下文信息（时间、线程、位置）组装并格式化成一条完整的日志草稿。
// Logger (管理者)：从 LogStream 处接收格式化好的日志草稿。它不关心格式化的细节，只根据日志级别等规则决定是否要将这份草稿“发表”。
// FileLog (执行者)：如果 Logger 决定“发表”，它就把最终的日志字符串交给 FileLog，由 FileLog 负责执行写入物理文件的具体操作。
// 这个三层结构（格式化 -> 决策 -> 执行）是这个日志库设计的精髓，它让每一部分都只关心自己的职责，实现了优秀的高内聚和低耦合。