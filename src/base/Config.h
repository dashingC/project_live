#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include "json/json.h"
#include "NonCopyable.h"
#include "Singleton.h"
#include "Logger.h"
#include "FileLog.h"
#include <mutex>

namespace tmms
{
    namespace base
    {

        struct LogInfo
        {
            LogLevel level;
            std::string path;
            std::string name;
            RotateType  rotate_type{kRotateNone};
        };

        using LogInfoPtr = std::shared_ptr<LogInfo>;

        class Config
        {
        public:
            Config() = default;
            ~Config() = default;

            bool LoadConfig(const std::string &file);
            
            LogInfoPtr &GetLogInfo();

            std::string name_;
            int32_t cpu_start_{0};
            int32_t thread_nums_{1};

            private:
            LogInfoPtr log_info_;
            bool ParseLogInfo(const Json::Value &root);


        };
        using ConfigPtr = std::shared_ptr<Config>;
        class ConfigMgr:public NonCopyable
        {

            public:
            ConfigMgr() = default;
            ~ConfigMgr() = default;

            bool LoadConfig(const std::string &file);
            ConfigPtr GetConfig();        

            private:
            ConfigPtr config_;
            std::mutex lock_;
        
        };
        #define sConfigMgr tmms::base::Singleton<tmms::base::ConfigMgr>::Instance()
    }

}