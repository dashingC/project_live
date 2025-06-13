#include "TaskMgr.h"
#include "TTime.h"
using namespace tmms::base;

void TaskMgr::OnWork()
{
    std::lock_guard<std::mutex> lock(lock_);
    int64_t now = TTime::NowMS();
    for (auto it = tasks_.begin(); it != tasks_.end();)
    {
        if ((*it)->When() <= now)
        {
            (*it)->Run();
            if ((*it)->When() < now)
            {
                it = tasks_.erase(it);
                continue;
            }
        }
        it++;
    }
}

bool TaskMgr::Add(TaskPtr &task)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = tasks_.find(task);
    if (iter != tasks_.end())
    {
        return false;
    }
    tasks_.emplace(task);
    return true;
}

bool TaskMgr::Del(TaskPtr &task)
{
    std::lock_guard<std::mutex> lock(lock_);
    tasks_.erase(task);
    return true;
}