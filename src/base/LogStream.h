#pragma once
#include "Logger.h"
#include <sstream>

namespace tmms
{
    namespace base
    {
        extern Logger *g_logger;
        class LogStream
        {
        public:
            LogStream(Logger *logger, const char *file, int line, LogLevel l, const char *func = nullptr);
            ~LogStream();

            template <typename T>
            LogStream &operator<<(const T &value)
            {
                stream_ << value;
                return *this;
            }

        private:
            std::ostringstream stream_;
            Logger *logger_{nullptr};
        };
    }
}
#define LOG_TRACE                                                  \
    if (tmms::base::g_logger->GetLogLevel() <= tmms::base::kTrace) \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kTrace, __func__)
#define LOG_DEBUG                                                  \
    if (tmms::base::g_logger->GetLogLevel() <= tmms::base::kDebug) \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kDebug, __func__)
#define LOG_INFO                                                  \
    if (tmms::base::g_logger->GetLogLevel() <= tmms::base::kInfo) \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kInfo)
#define LOG_WARN \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kWarn)
#define LOG_ERROR \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kError)
// 这个宏定义用于在日志流中添加一个值，并返回当前日志流对象的引用，以便可以链式调用。
//__func__是一个编译器在编译时会自动替换的变量。它会被替换成一个包含当前函数名称的字符串。
//例如，如果在 int main() 函数中使用了 __func__，它的值就是 "main"