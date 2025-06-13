#include "Singleton.h"
#include <iostream>
using namespace tmms::base;

class A : public NonCopyable
{
private:
    A() = default;
    ~A() = default;
    friend class Singleton<A>;

public:
    void Print()
    {
        std::cout << ("TestSingleton Instance: \n") << std::endl;
    }
};

#define sA tmms::base::Singleton<A>::Instance()
// int main(int argc, const char **argv)
// {
//     sA->Print();

//     return 0;
// }