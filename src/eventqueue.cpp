#include "eventqueue.h"

namespace toggl {

EventQueue::EventQueue(Context *parent)
    : parent(parent)
{

}

EventQueue::~EventQueue() {
    std::unique_lock<std::recursive_mutex> lock(queueLock);
    if (workThread.get_id() == std::thread::id()) {
        lock.unlock();
        terminate = true;
        cv.notify_all();
        workThread.join();
    }
}

void EventQueue::start() {
    if (workThread.get_id() == std::thread::id())
        workThread = std::thread(&EventQueue::threadLoop, this);
    else
        logger.warning("A worker thread is already running!");
}

void EventQueue::schedule(std::chrono::time_point<std::chrono::system_clock> time, std::function<void ()> event) {
    std::unique_lock<std::recursive_mutex> lock(queueLock);
    queue.emplace(time, event);
    lock.unlock();
    cv.notify_all();
}

void EventQueue::schedule(std::chrono::seconds after, std::function<void ()> event) {
    schedule(std::chrono::system_clock::now() + after, event);
}

void EventQueue::schedule(std::function<void ()> event) {
    schedule(std::chrono::time_point<std::chrono::system_clock> {}, event);
}

void EventQueue::threadLoop() {
    while (true) {
        std::unique_lock<std::recursive_mutex> lock(queueLock);
        auto time = std::chrono::system_clock::now();
        if (terminate)
            return;
        while (!queue.empty() && queue.begin()->first <= time) {
            if (terminate)
                return;
            logger.log("The time is now", std::to_string(time.time_since_epoch().count()));
            logger.log("The event time is", std::to_string(queue.begin()->first.time_since_epoch().count()));
            queue.begin()->second();
            queue.erase(queue.begin());
        }
        if (queue.begin() != queue.end()) {
            logger.log("Waiting for an event");
            auto nextTime = queue.begin()->first;
            lock.unlock();
            std::unique_lock<std::mutex> cvLock(cvMutex);
            cv.wait_until(cvLock, nextTime);
        }
        else {
            logger.log("Waiting before somebody wakes me up");
            lock.unlock();
            std::unique_lock<std::mutex> cvLock(cvMutex);
            cv.wait(cvLock);
        }
    }
}

} // namespace toggl
