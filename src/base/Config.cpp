#include "Config.h"

using namespace tmms::base;

bool Config::LoadConfig(const std::string& file)
{

}
bool  Config::ParseLogInfo(const Json::Value& root)//解析
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
LogInfoPtr& Config::GetLogInfo()
{
    return log_info_;
}
