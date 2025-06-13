#include "LogStream.h"
#include "TTime.h"
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

using namespace tmms::base;
Logger *tmms::base::g_logger = nullptr;
static thread_local pid_t thread_id = 0;

const char *log_stream[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};



 LogStream::LogStream(Logger *logger, const char *file, int line, LogLevel l, const char *func)
 : logger_(logger) 
 {
    const char *filename = strrchr(file, '/');
    if (filename == nullptr)
    {
        filename = file; // 说明没有找到 '/', 使用整个文件名
    }
    else
    {
        filename++; // 说明找到了 '/', 跳过 '/'
    }
    stream_ << TTime::ISOTime()<<"  ";
    if(thread_id == 0)
    {
        thread_id = static_cast<pid_t>(::syscall(SYS_gettid)); // 获取当前线程ID
    }
    stream_<<thread_id<<" ";
    stream_<< log_stream[l];
    stream_ << " [" << filename << ":" << line << "] ";
    if(func)
    {
        stream_ << " ["<<func << "] ";
    }

 }
LogStream::~LogStream()
{
    stream_ << "\n";
    if (logger_)
    {
        logger_->Write(stream_.str());
    }

}

//这个函数的作用是将传入的值转换为字符串并添加到日志流中，返回的值是当前日志流对象的引用，以便可以链式调用。
// template <typename T> LogStream & LogStream::operator<<(const T &value)
// { 
//     stream_ << value;
//     return *this;
// }
