#include "FileMgr.h"
#include "TTime.h"
#include "StringUtils.h"
#include <sstream>
using namespace tmms::base;

namespace
{
    static tmms::base::FileLogPtr file_log_nullptr;
}

// 检测是否跨天或跨小时或者跨分钟
void FileMgr::OnCheck()
{
    bool day_change{false};
    bool hour_change{false};
    bool minute_change{false};
    int year = 0, month = 0, day = -1, hour = -1, minute = 0, second = 0;
    TTime::Now(year, month, day, hour, minute, second);
    if (last_day_ == -1)
    {
        last_day_ = day;
        last_hour_ = hour;
        last_minute_ = minute;
        last_month_ = month;
        last_year_ = year;
    }

    if (last_minute_ != minute)
        minute_change = true;
    if (last_day_ != day)
        day_change = true;
    if (last_hour_ != hour)
        hour_change = true;
    if (!day_change && !hour_change)
        return;

    std::lock_guard<std::mutex> lk(lock_);
    for (auto &l : logs_) // 遍历所有日志对象，判断是否需要切分
    {
        
        if (minute_change && l.second->GetRotateType() == kRotateMinute) // 判断是否跨分钟 & 日志是否设置为“按分钟切分”
        {
            RotateMinutes(l.second);
        }

        if (hour_change && l.second->GetRotateType() == kRotateHour) // 判断是否跨小时 & 日志是否设置为“按小时切分”
        {
            RotateHours(l.second);
        }
        if (day_change && l.second->GetRotateType() == kRotateDaily) // 判断是否跨天 & 日志是否设置为“按天切分”
        {
            RotateDays(l.second);
        }
    }

    last_day_ = day;
    last_hour_ = hour;
    last_month_ = month;
    last_year_ = year;
}
FileLogPtr FileMgr::GetFileLog(const std::string &file_name)// 获取文件日志对象，如果不存在则创建一个新的
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = logs_.find(file_name);
    if (iter != logs_.end())
    {
        return iter->second;// 如果日志已存在，则直接返回iter的第二个值，即FileLogPtr对象
    }

    // 如果日志不存在，则创建一个新的日志对象
    //语法详细解释：
    // 基本语法是 std::make_shared<要创建的类型>(给构造函数的参数...)
    // 在 std::make_shared<FileLog>() 中，跟在类型后面的括号 () 就是用来传递参数给 FileLog 类的构造函数的
    FileLogPtr log = std::make_shared<FileLog>();
    if (!log->Open(file_name))
    {
        return file_log_nullptr; // 打开日志文件失败
    }
    logs_.emplace(file_name, log);// 将新创建的日志对象插入到logs_哈希表中，键为file_name，值为log
    return log;
}
void FileMgr::RemoveFileLog(const FileLogPtr &log)
{
    std::lock_guard<std::mutex> lk(lock_);
    logs_.erase(log->FilePath());
}
void FileMgr::RotateDays(const FileLogPtr &file)
{
    if (file->Filesize() > 0)
    {
        char buf[128] = {
            0,
        };
        sprintf(buf, "%04d-%02d-%02d", last_year_, last_month_, last_day_);
        std::string file_path = file->FilePath();
        std::string path = StringUtils::FilePath(file_path);
        std::string file_name = StringUtils::FileName(file_path);
        std::string file_ext = StringUtils::Extension(file_path);

        std::ostringstream ss;
        ss << path
           << file_name
           << buf
           << file_ext;
        file->Rotate(ss.str());
    }
}
void FileMgr::RotateHours(const FileLogPtr &file)
{
    if (file->Filesize() > 0)
    {
        char buf[128] = {
            0,
        };
        sprintf(buf, "%04d-%02d-%02dT%02d%02d", last_year_, last_month_, last_day_, last_hour_, last_minute_);
        std::string file_path = file->FilePath();
        std::string path = StringUtils::FilePath(file_path);
        std::string file_name = StringUtils::FileName(file_path);
        std::string file_ext = StringUtils::Extension(file_path);

        std::ostringstream ss;
        ss << path
           << file_name
           << buf
           << file_ext;
        file->Rotate(ss.str());
    }
}
void FileMgr::RotateMinutes(const FileLogPtr &file)
{
    if (file->Filesize() > 0)
    {
        char buf[128] = {
            0,
        };
        sprintf(buf, "%04d-%02d-%02dT%02d", last_year_, last_month_, last_day_, last_hour_);
        std::string file_path = file->FilePath();
        std::string path = StringUtils::FilePath(file_path);
        std::string file_name = StringUtils::FileName(file_path);
        std::string file_ext = StringUtils::Extension(file_path);

        std::ostringstream ss;
        ss << path
           << file_name
           << buf
           << file_ext;
        file->Rotate(ss.str());
    }
}