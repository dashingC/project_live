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
// 有限状态机的字符串分割，将字符串s按照指定的分隔符delimiter进行分割
std::vector<std::string> StringUtils::SplitStringWithFSM(const string& s, const char delimiter)
{
    // 定义状态机的状态枚举值
    enum
    {
        kStateInit = 0,             // 初始状态
        kStateNormal = 1,           // 正常状态，处理非分隔符字符
        kStateDelimiter = 2,        // 分隔符状态，处理分隔符
        kStateEnd = 3               // 结束状态
    };

    // result 用于存储分割后的子字符串，临时变量定义在返回值的内存空间中，不会产生额外开销
    std::vector<std::string> result;
    
    // 初始化状态为初始状态
    int state = kStateInit;

    // 临时字符串变量，用于存储当前解析的子字符串
    std::string tmp;

    // 将状态设置为正常状态，准备开始解析字符串
    state = kStateNormal;

    // 遍历输入字符串 s
    for (int pos = 0; pos < s.size();)
    {
        // 当前状态为正常状态，处理非分隔符字符
        if (state == kStateNormal)
        {
            // 如果当前字符是分隔符
            if (s.at(pos) == delimiter)
            {
                // 状态转移到分隔符状态
                state = kStateDelimiter;
                // 跳过本次循环，不增加 pos 以便在下次处理
                continue;
            }

            // 如果当前字符不是分隔符，将其添加到临时字符串 tmp 中
            tmp.push_back(s.at(pos));
            // 移动到下一个字符
            pos++;
        }
        // 当前状态为分隔符状态
        else if (state == kStateDelimiter)
        {
            // 将临时字符串 tmp 存入结果向量 result 中
            result.push_back(tmp);
            // 清空临时字符串以存储下一个子字符串
            tmp.clear();
            // 状态转移回正常状态
            state = kStateNormal;
            // 继续处理下一个字符
            pos++;
        }
    }

    // 如果循环结束后临时字符串 tmp 中还有内容，说明最后一个子字符串没有被添加到 result 中
    if (!tmp.empty())
    {
        // 将最后一个子字符串添加到结果向量中
        result.push_back(tmp);
    }

    // 返回分割后的结果向量
    return result;
}