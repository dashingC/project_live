#include "TTime.h"
#include <sys/time.h>

using namespace tmms::base;
int64_t TTime::NowMS()//将当前时间转换为毫秒
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int64_t TTime::Now()//将当前时间转换为秒
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

//将当前时间转换为年月日时分秒
int64_t TTime::Now(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    year = tm.tm_year + 1900;
    month = tm.tm_mon + 1;
    day = tm.tm_mday;
    hour = tm.tm_hour;
    minute = tm.tm_min;
    second = tm.tm_sec;
    return t;
}

//将当前时间转换为ISO 8601格式的字符串
std::string TTime::ISOTime()
{
    struct timeval tv;
    struct tm tm;
    gettimeofday(&tv, NULL);
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    char buf[128] = {0};
    auto n = sprintf(buf, "%4d-%02d-%02dT%02d:%02d:%02d",
                     tm.tm_year + 1900,
                     tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return std::string(buf, buf + n);
}