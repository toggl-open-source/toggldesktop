#ifndef SRC_EVENT_QUEUE_H
#define SRC_EVENT_QUEUE_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <chrono>

namespace toggl {

class EventQueue;

class Event {
public:
    Event();
    virtual ~Event();

    virtual void execute();
};

class EventQueue {
public:
    using time_clock = std::chrono::steady_clock;
    using time_duration_ms = std::chrono::microseconds;
    using time_point = std::chrono::time_point<time_clock>;

    EventQueue();

    void clear();

    void wakeUp();

    static time_point now();

    void schedule(Event *event, int64_t in_ms = 0);
    void schedule(Event *event, const time_point &at);

private:
    [[noreturn]] void execute();

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::multimap<time_point, Event*, std::less<time_point>> queue_;
};

};

#endif // SRC_EVENT_QUEUE_H
