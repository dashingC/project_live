#pragma once

#include <string>
#include <vector>

namespace tmms
{
    namespace base
    {
        using std::string;

        class StringUtils
        {
            public:
            static bool StartWith(const string &s ,const string &sub);           //检查字符是不是以sub开头
            static bool EndWith(const string &s ,const string &sub);             //检查字符是不是以sub结尾
            static string FilePath(const string &path );                        //返回当前文件所在的路径
            static string FileNameExt(const string &path );                    //返回文件名
            static string FileName(const string &path );                       //返回文件名字不带后缀
            static string Extension(const string &path );                     //返回文件后缀名
            static std::vector<string> SplitString(const string &s, const string &delimiter);//分割字符串

        };
    }
}

