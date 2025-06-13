#pragma once
#include "NonCopyable.h"
#include "FileLog.h"
#include <string>

//这个头文件是日志记录器的声明文件，用于定义日志记录器的接口和日志级别枚举。
namespace tmms
{
    namespace base
    {
        enum LogLevel
        {
            kTrace,
            kDebug,
            kInfo,
            kWarn,
            kError,
            kMaxNumOfLogLevels,
        };

        class Logger : public NonCopyable
        {
        public:
            Logger(const FileLogPtr &log);
            ~Logger() = default;

            void SetLogLevel(const LogLevel &level);
            LogLevel GetLogLevel() const;
            void Write(const std::string &msg);

        private:
            LogLevel level_{kDebug};
            FileLogPtr log_; 
        };
    }
}