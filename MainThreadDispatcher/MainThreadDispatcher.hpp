#pragma once

#include <functional>
#include <vector>

struct DelayedTask
{
    std::function<void()> func;
    long delayMs;
    unsigned long scheduledTime;
};

class MainThreadDispatcher
{
public:
    static void Loop()
    {
        if (_tasks().empty())
            return;
        std::vector<DelayedTask> currentTasks;
        currentTasks.insert(currentTasks.end(), _tasks().begin(), _tasks().end());
        unsigned long currentTime = millis();
        for (auto it = currentTasks.begin(); it != currentTasks.end();)
        {
            if (currentTime - it->scheduledTime < it->delayMs)
            {
                ++it;
                continue;
            }
            it->func();
            yield();
            // remove it from original tasks list
            auto originalIt = std::find_if(_tasks().begin(), _tasks().end(), [it](const DelayedTask &task) { return task.scheduledTime == it->scheduledTime && task.delayMs == it->delayMs; });
            if (originalIt != _tasks().end())
                _tasks().erase(originalIt);
            it = currentTasks.erase(it);
        }
    }

    static void Dispatch(std::function<void()> func, long delayMs = 0)
    {
        DelayedTask task;
        task.func = func;
        task.delayMs = delayMs;
        task.scheduledTime = millis();
        _tasks().push_back(task);
    }
    
    static void Dispatch_Ptr(void (*func)(), long delayMs = 0)
    {
        Dispatch(std::function<void()>(func), delayMs);
    }

private:
    static std::vector<DelayedTask> &_tasks() 
    {
        static std::vector<DelayedTask> instance;
        return instance;
    }
};
