#pragma once
#include "base/NonCopyable.h"
#include "EventLoop.h"
#include <vector>
#include "EventLoopThread.h"
#include <memory>
#include <atomic>

namespace tmms{
    namespace network{

        using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
        class EventLoopThreadPool:public base::NonCopyable{
        public:
            EventLoopThreadPool(int thread_num, int start=0, int cpus=4);
            ~EventLoopThreadPool();

            // 返回所有的事件循环
            std::vector<EventLoop*> GetLoops() const;
            // 获取事件循环的接口
            EventLoop * GetNextLoop();
            // 返回线程数量
            size_t Size();
            // 启动线程池
            void Start();

        private:
            std::vector<EventLoopThreadPtr> threads_; 
            std::atomic<int32_t> loop_index_{0};//用来指示我们取到的loop是哪个
        };
    }
}