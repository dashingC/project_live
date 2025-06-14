#include "base/Config.h"
#include "base/LogStream.h"
#include "base/FileMgr.h"
#include "base/TaskMgr.h"

#include <iostream>
#include <thread>

using namespace tmms::base;

int main(int argc, const char **argv)
{
    if (!sConfigMgr->LoadConfig("../config/config.json"))
    {
        std::cerr << "load config failed!" << std::endl;
        return -1;
    }

    ConfigPtr config = sConfigMgr->GetConfig();
    LogInfoPtr log_info = config->GetLogInfo();
    std::cout << " Level: " << log_info->level
              << " path: " << log_info->path
              << " Name: " << log_info->name
              << " RotateType: " << log_info->rotate_type
              << std::endl;
    FileLogPtr log = sFileMgr->GetFileLog(log_info->path + log_info->name);
    if (!log)
    {
        std::cerr << "Failed to create log file" << std::endl;
        return -1;
    }
    log->SetRotate(log_info->rotate_type);
    g_logger = new Logger(log);
    g_logger->SetLogLevel(log_info->level);


    TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task)
                                           {
                                            sFileMgr->OnCheck();
                                            task->Restart(); },
                                           1000);
    while (1)
    {

        sTaskMgr->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}