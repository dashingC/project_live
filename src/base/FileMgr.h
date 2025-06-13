#pragma once

#include "FileLog.h"
#include "Singleton.h"
#include <memory>
#include <unordered_map>
#include <mutex>

namespace tmms
{
    namespace base
    {

        class FileMgr : public NonCopyable
        {
        public:
            FileMgr() = default;
            ~FileMgr() = default;

            void OnCheck();// 检测是否跨天或跨小时或者跨分钟
            FileLogPtr GetFileLog(const std::string &file_name);// 获取文件日志对象，如果不存在则创建一个新的
            void RemoveFileLog(const FileLogPtr &log);// 删除指定的文件日志对象
            void RotateDays(const FileLogPtr &file);// 根据天来切分日志
            void RotateHours(const FileLogPtr &file);// 根据小时来切分日志
            void RotateMinutes(const FileLogPtr &file);// 根据分钟来切分日志

        private:
            std::unordered_map<std::string, FileLogPtr> logs_;// 存储文件日志对象的哈希表，键为文件名，值为文件日志对象指针
            std::mutex lock_;
            int last_minute_{-1};           
            int last_hour_{-1};
            int last_day_{-1};
            int last_month_{-1};
            int last_year_{-1};
        };
    }
} // namespace tmms::base
//定义一个宏来简化对 FileMgr 实例的访问
#define sFileMgr tmms::base::Singleton<tmms::base::FileMgr>::Instance()