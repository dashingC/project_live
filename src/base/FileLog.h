 #pragma once
 #include <string>
#include <memory>
namespace tmms
{
    namespace base
    {
        enum RotateType
        {
            kRotateNone, // 不切分
            kRotateMinute, // 按分钟切分
            kRotateHour, // 按小时切分
            kRotateDay, // 按天切分                   
        };
        
        class FileLog
        {
            public:
            FileLog()= default;
            ~FileLog() = default;

            bool Open(const std::string &filePath);// 打开日志文件
            size_t WriteLog(const std::string &msg);// 写入日志
            void Rotate(const std::string &file);// 切分日志文件
            void SetRotate(RotateType type);// 设置切分类型
            RotateType GetRotateType() const;// 获取当前的切分类型
            int64_t Filesize() const;// 获取文件大小
            std::string FilePath() const;// 返回文件路径

            private:
            int fd_{-1};
            std::string file_path_;
            RotateType rotate_type_{kRotateNone};

        };
        using FileLogPtr = std::shared_ptr<FileLog>;
        //这是 C++11 引入的为类型创建别名的新语法，功能上类似于旧式的 typedef。它让代码更易读。
        // 意思是：”在我的程序里，请把 FileLogPtr 这个名字当作 std::shared_ptr<FileLog> 的同义词来使用

    }
}