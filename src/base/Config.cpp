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
        std::string level = levelObj.asString();
        if (level == "TRACE")
        {
            log_info_->level = kTrace;
        }
        else if (level == "DEBUG")
        {
            log_info_->level = kDebug;
        }
        else if (level == "INFO")
        {
            log_info_->level = kInfo;
        }
        else if (level == "WARN")
        {
            log_info_->level = kWarn;
        }
        else if (level == "ERROR")
        {
            log_info_->level = kError;
        }
        else
        {
            LOG_ERROR << "Invalid log level: " << level;
            return false;
        }
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

    Json::Value rtObj = root["rotate"];
    if (!rtObj.isNull())
    {
        std::string rt = rtObj.asString();
        if (rt == "DAY")
        {
            log_info_->rotate_type = kRotateDay;
        }
        else if (rt == "HOUR")
        {
            log_info_->rotate_type = kRotateHour;
        }
        else
        {
            LOG_ERROR << "Invalid log rotate type: " << rt;
            return false;
        }
    }
}
LogInfoPtr &Config::GetLogInfo()
{
    return log_info_;
}
bool ConfigMgr::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "load config file: " << file;
    ConfigPtr config = std::make_shared<Config>();
    if (config->LoadConfig(file))
    {
        std::lock_guard<std::mutex> lk(lock_);
        config_ = config;
        LOG_DEBUG << "config loaded successfully.";
        return true;
    }
    return false;
}
ConfigPtr ConfigMgr::GetConfig()
{
    std::lock_guard<std::mutex> lock(lock_);
    return config_;
}