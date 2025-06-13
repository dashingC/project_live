#include "FileLog.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

using namespace tmms::base;

bool FileLog::Open(const std::string &filePath)
{
    file_path_ = filePath;
    int fd = ::open(file_path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, DEFFILEMODE);
    if (fd < 0)
    {
        std::cout << " open file failed,path:" << filePath << std::endl;
        return false; // 打开文件失败
    }
    fd_ = fd; // 保存文件描述符
    return true;
}

size_t FileLog::WriteLog(const std::string &msg) // 写入日志
{
    int fd = fd_ == -1 ? 1 : fd_;
    return ::write(fd, msg.data(), msg.size());
}

void FileLog::Rotate(const std::string &file) // 切分日志文件
{
    if (file_path_.empty())
    {
        return;
    }
    int ret = ::rename(file_path_.c_str(), file.c_str());
    if (ret!=0)
    {
        std::cerr << " rename file failed,old path:" << file_path_ << ",new path:" << file << std::endl;
        return;
    }
    
    int fd = ::open(file_path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, DEFFILEMODE);
    if (fd < 0)
    {
        std::cout << " open file failed,path:" << file << std::endl;
        return;
    }
    ::dup2(fd, fd_); // 将新的文件描述符(fd)复制到原来的文件描述符fd_
    ::close(fd);     // 关闭新打开的文件描述符`
}

void FileLog::SetRotate(RotateType type) // 设置切分类型
{
    rotate_type_ = type;
}
RotateType FileLog::GetRotateType() const // 获取当前的切分类型
{
    return rotate_type_;
}
int64_t FileLog::Filesize() const // 获取文件大小
{
    return ::lseek64(fd_, 0, SEEK_END);
}
std::string FileLog::FilePath() const // 返回文件路径
{
    return file_path_;
}