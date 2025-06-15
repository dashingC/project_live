#include "TimingWheel.h"
#include "network/base/Network.h"
using namespace tmms::network;

TimingWheel::TimingWheel() : wheels_(4)
{
    wheels_[kTimingWheelSecond].resize(60);
    wheels_[kTimingWheelMinute].resize(60);
    wheels_[kTimingWheelHour].resize(24);
    wheels_[kTimingWheelDay].resize(30);
}

TimingWheel::~TimingWheel()
{
}

void TimingWheel::InsertEntry(uint32_t delay, EntryPtr entryPtr)
{
    if (delay <= 0)
    {
        entryPtr.reset();
    }
    if (delay < kTimingMinute)
    {
        InsertSecondEntry(delay, entryPtr);
    }
    else if (delay < kTimingHour)
    {
        InsertMinuteEntry(delay, entryPtr);
    }
    else if (delay < kTimingDay)
    {
        InsertHourEntry(delay, entryPtr);
    }
    else
    {
        auto day = delay / kTimingDay;
        if (day > 30)
        {
            NETWORK_ERROR << "NO SUPPORT ! day is too long. day:" << day;
            return;
        }
        InsertDayEntry(delay, entryPtr);
    }
}

void TimingWheel::OnTimer(int64_t now)
{
    if (last_ts_ == 0)
    {
        last_ts_ = now;
    }
    if (now - last_ts_ < 1000)
    {
        return;
    }
    last_ts_ = now;
    tick_++;
    PopUp(wheels_[kTimingWheelSecond]);
    if (tick_ % kTimingMinute == 0)
    {
        PopUp(wheels_[kTimingWheelMinute]);
    }
    else if (tick_ % kTimingHour == 0)
    {
        PopUp(wheels_[kTimingWheelHour]);
    }
    else if (tick_ % kTimingDay == 0)
    {
        PopUp(wheels_[kTimingWheelDay]);
    }
}
void TimingWheel::PopUp(Wheel &bq)
{
    // 1. 准备一个临时的空槽位（空的“任务袋”）
    WheelEntry tmp;
    // 2. 将双端队列中的第一个槽位（任务袋）中的所有任务转移到临时槽位中，轮子上的槽位为空
    bq.front().swap(tmp);
    // 3. 将旧槽位（现在已经是空的）从轮子头部移除
    bq.pop_front();
    // 4. 在轮子尾部补充一个新的空槽位
    bq.push_back(WheelEntry());
}

void TimingWheel::RunAfter(double delay, const Func &cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([cb]()
                                                               { cb(); });
    InsertEntry(delay, cbEntry);
}

void TimingWheel::RunAfter(double delay, Func &&cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([cb]()
                                                               { cb(); });
    InsertEntry(delay, cbEntry);
}

void TimingWheel::RunEvery(double inerval, const Func &cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([this, inerval, cb]()
                                                               {
        cb();
        RunEvery(inerval, cb); });
    InsertEntry(inerval, cbEntry);
}

void TimingWheel::RunEvery(double inerval, Func &&cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([this, inerval, cb]()
                                                               {
        cb();
        RunEvery(inerval, cb); });
    InsertEntry(inerval, cbEntry);
}

// 插入秒
void TimingWheel::InsertSecondEntry(uint32_t delay, EntryPtr entryPtr)
{
    wheels_[kTimingWheelSecond][delay - 1].emplace(entryPtr);
}

// 插入分钟
void TimingWheel::InsertMinuteEntry(uint32_t delay, EntryPtr entryPtr)
{

    auto minute = delay / kTimingMinute;
    auto second = delay % kTimingMinute;
    CallbackEntryPtr newEntryPtr = std::make_shared<CallbackEntry>([this, second, entryPtr]()
                                                                   { InsertEntry(second, entryPtr); });
    wheels_[kTimingWheelMinute][minute - 1].emplace(newEntryPtr);
}
// 插入小时
void TimingWheel::InsertHourEntry(uint32_t delay, EntryPtr entryPtr)
{
    auto hour = delay / kTimingHour;
    auto second = delay % kTimingHour;
    CallbackEntryPtr newEntryPtr = std::make_shared<CallbackEntry>([this, second, entryPtr]()
                                                                   { InsertEntry(second, entryPtr); });
    wheels_[kTimingWheelHour][hour - 1].emplace(newEntryPtr);
}
// 插入天
void TimingWheel::InsertDayEntry(uint32_t delay, EntryPtr entryPtr)
{
    auto day = delay / kTimingDay;
    auto second = delay % kTimingDay;
    CallbackEntryPtr newEntryPtr = std::make_shared<CallbackEntry>([this, second, entryPtr]()
                                                                   { InsertEntry(second, entryPtr); });
    wheels_[kTimingWheelDay][day - 1].emplace(newEntryPtr);
}