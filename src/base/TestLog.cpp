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
    FileLogPtr log = sFileMgr->GetFileLog("test.log");
    log->SetRotate(kRotateMinute);

    tmms::base::g_logger = new Logger(log);

    tmms::base::g_logger->SetLogLevel(kWarn);

    TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task)
                                           {
                                            sFileMgr->OnCheck();
                                            task->Restart();
                                            }, 
                                            1000);
    TestLog();
    return 0;
}