#ifndef SRC_EVENT_QUEUE_H
#define SRC_EVENT_QUEUE_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <set>
#include <chrono>

namespace toggl {

class EventQueue;
class Context;

class Event {
public:
    friend class EventQueue;
    Event(EventQueue *queue);
    virtual ~Event();

protected:
    void schedule(int64_t in_ms);
    void unschedule();
    virtual void execute();

private:
    EventQueue *queue_;
};

class EventQueue {
public:
    using time_clock = std::chrono::steady_clock;
    using time_duration_ms = std::chrono::microseconds;
    using time_point = std::chrono::time_point<time_clock>;

    EventQueue(Context *context);

    void clear();

    void wakeUp();

    static time_point now();

    void schedule(Event *event, int64_t in_ms = 0);
    void schedule(Event *event, const time_point &at);
    void unschedule(Event *event);

private:
    [[noreturn]] void execute();

    Context *context_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::set<Event *> managed_events_;
    std::multimap<time_point, Event*, std::less<time_point>> queue_;
};

};

#endif // SRC_EVENT_QUEUE_H
