#include "Config.h"
#include "LogStream.h"

using namespace tmms::base;

bool Config::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "config file: " << file;
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(file, root))
    {
        LOG_ERROR << "Failed to parse config file: " << file;
        return false;
    }

    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        name_ = nameObj.asString();
    }

    Json::Value cpusObj = root["cpu_start"];
    if (!cpusObj.isNull())
    {
        cpu_start_ = cpusObj.asInt();
    }

    Json::Value threadsObj = root["cpu_start"];
    if (!cpusObj.isNull())
    {
        thread_nums_ = threadsObj.asInt();
    }

    Json::Value logObj = root["log"];
    if (!logObj.isNull())
    {
        ParseLogInfo(logObj);
    }

    return true;
}
bool Config::ParseLogInfo(const Json::Value &root) // 解析
{
    log_info_ = std::make_shared<LogInfo>();

    Json::Value levelObj = root["level"];
    if (!levelObj.isNull())
    {
        log_info_->level = levelObj.asString();
    }

    Json::Value pathObj = root["path"];
    if (!pathObj.isNull())
    {
        log_info_->path = pathObj.asString();
    }

    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        log_info_->name = nameObj.asString();
    }
}
LogInfoPtr &Config::GetLogInfo()
{
    return log_info_;
}
