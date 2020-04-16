#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

#include "logger.h"

namespace toggl {

class Context;

class EventQueue
{
public:
    EventQueue(Context *parent);

    void start();

    // TODO rewrite this into a template method
    void schedule(std::chrono::time_point<std::chrono::system_clock> time, std::function<void(void)> event);
    void schedule(std::chrono::seconds after, std::function<void(void)> event);
    void schedule(std::function<void(void)> event);

private:
    [[noreturn]] void threadLoop();

    std::multimap<std::chrono::time_point<std::chrono::system_clock>, std::function<void(void)>> queue;
    std::recursive_mutex queueLock;
    std::condition_variable cv;
    std::mutex cvMutex;

    Context *parent { nullptr };
    Logger logger { "EventQueue" };

    std::thread workThread;
};

} // namespace toggl

#endif // EVENTQUEUE_H
