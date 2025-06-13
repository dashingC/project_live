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
            kRotateDaily, // 按天切分                   
        };
        
        class FileLog
        {
            public:
            FileLog()= default;
            ~FileLog() = default;

            bool Open(const std::string &filePath);
            size_t WriteLog(const std::string &msg);
            void Rotate(const std::string &file);
            void SetRotate(RotateType type);
            RotateType GetRotateType() const;
            int64_t Filesize() const;
            std::string FilePath() const;

            private:
            int fd_{-1};
            std::string file_path_;
            RotateType rotate_type_{kRotateNone};

        };
        using FileLogPtr = std::shared_ptr<FileLog>;

    }
}