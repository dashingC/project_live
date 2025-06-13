#pragma once
#include <cstdint>
#include <memory>
#include <functional>

namespace tmms
{
    namespace base
    {
        class Task;
        using TaskPtr = std::shared_ptr<Task>;
        using TaskCallback = std::function<void(const TaskPtr &)>;
        class Task:public std::enable_shared_from_this<Task>
        {
        public:
            Task(const TaskCallback &cb, int64_t interval_);
            Task(const TaskCallback &&cb, int64_t interval_);
            void Run();
            void Restart();
            int64_t When() const
            {
                return when_;
            }

        private:
            
            int64_t interval_{0};
            int64_t when_{0};
            TaskCallback cb_;
        };
    }
} // namespace tmms::base