#include "StringUtils.h"

using namespace tmms::base;

bool StringUtils::StartWith(const string &s, const string &sub)
{
    if (sub.empty())
        return true;
    if (s.empty())
        return false;

    auto len = s.size();
    auto slen = sub.size();

    if (slen > len)
        return false;

    return s.compare(0, slen, sub) == 0;
}
bool StringUtils::EndWith(const string &s, const string &sub)
{
    if (sub.empty())
        return true;
    if (s.empty())
        return false;

    auto len = s.size();
    auto slen = sub.size();

    if (slen > len)
        return false;

    return s.compare(len - slen, slen, sub) == 0;
}
string StringUtils::FilePath(const string &path)
{
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
        return path.substr(0, pos);
    else
        return "./";
}
string StringUtils::FileNameExt(const string &path)
{
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        if (pos + 1 < path.size())
        {
            return path.substr(pos + 1);
        }
    }
    return path;
}
string StringUtils::FileName(const string &path)
{
    string file_name = FileNameExt(path);
    auto pos = file_name.find_last_of(".");
    if (pos != std::string::npos)
    {
        if (pos != 0)
        {
            return file_name.substr(0, pos);
        }
    }
    return file_name;
}
string StringUtils::Extension(const string &path) 
{
    string file_name = FileNameExt(path);
    auto pos = file_name.find_last_of(".");
    if (pos != std::string::npos)
    {
        if (pos != 0 && pos+1<file_name.size())
        {
            return file_name.substr(pos+1);
        }
    }
    return std::string();
}
std::vector<string> SplitString(const string &s, const string &delimiter)
{
    //这个函数的具体作用是将字符串s按照指定的分隔符delimiter进行分割，并返回一个字符串向量，
    //如果s是"hello,world,example"而delimiter是","，那么返回的向量将包含{"hello", "world", "example"}。
    if (s.empty() || delimiter.empty())
        return {};
    std::vector<string> result;
    size_t last = 0;
    size_t next = 0;
    while ((next = s.find(delimiter, last)) != string::npos) 
    {
        if (next > last) 
        {
            result.emplace_back(s.substr(last, next - last));
        }
        last = next + delimiter.size();
    }
    if (last < s.size()) 
    {
        result.emplace_back(s.substr(last));
    }
    return result;
}