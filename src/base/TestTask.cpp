#include "TaskMgr.h"
#include "TTime.h"
#include <iostream>
#include <thread>

using namespace tmms::base;

void TestTask()
{
    TaskPtr task1= std::make_shared<Task>([](const TaskPtr &task) 
    {
        std::cout << "Task 1 interval: " << 1000 <<"now:"<<TTime::NowMS()<< std::endl;
    }, 1000);

    TaskPtr task2 = std::make_shared<Task>([](const TaskPtr &task) 
    {
        std::cout << "Task 2 interval:  " << 1000<<"now:"<<TTime::NowMS() << std::endl;
        task->Restart(); 
    }, 1000);

    TaskPtr task3 = std::make_shared<Task>([](const TaskPtr &task) 
    {
        std::cout << "Task 3 interval: " << 500 <<"now:"<<TTime::NowMS()<< std::endl;
        task->Restart(); 
    }, 500); 

    TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task) 
    {
        std::cout << "Task 4 interval: " << 30000 <<"now:"<<TTime::NowMS()<< std::endl;
        task->Restart(); 
    }, 3000);
    STaskMgr->Add(task1);
    STaskMgr->Add(task2);
    STaskMgr->Add(task3);
    STaskMgr->Add(task4);
}    
// int main()
// {
//     TestTask();
//     while(1)
//     {
//         STaskMgr->OnWork();
//         std::this_thread::sleep_for(std::chrono::milliseconds(50));

//     }
//     return 0;
// }
