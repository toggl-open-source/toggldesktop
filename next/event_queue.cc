
#include "event_queue.h"

#include <iostream>

namespace toggl {

EventQueue::EventQueue()
    : thread_(std::thread(&EventQueue::execute, this))
{
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
}

void EventQueue::clear() {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }
    condition_.notify_one();
}

EventQueue::time_point EventQueue::now() {
    return time_clock::now();
}

void EventQueue::execute() {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
        if (queue_.empty())
            condition_.wait(lock);
        else
            while (true) {
            auto front = queue_.begin();
            if (front == queue_.end())
                break;
            else if (front->first <= now()) {
                front->second->execute();
                queue_.erase(front);
            }
            else {
                condition_.wait_until(lock, front->first);
            }
        }
    }
}

void EventQueue::schedule(Event *event, int64_t in_ms) {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
    if (in_ms <= 0) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto newTime = now() - std::chrono::hours(1);
            queue_.insert(std::make_pair(newTime, event));
        }
        wakeUp();
    }
    else {
        auto newTime = now() + std::chrono::milliseconds(in_ms);
        schedule(event, newTime);
    }
}

void EventQueue::schedule(Event *event, const time_point &at) {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
    time_point first;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        first = queue_.begin()->first;
        queue_.emplace(at, event);
    }
    if (first > at) {
        wakeUp();
    }
}

void EventQueue::wakeUp() {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
    condition_.notify_one();
}

Event::Event() {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
}

Event::~Event() {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< std::endl;
}

void Event::execute() {
    std::cerr << std::this_thread::get_id() << " " << __PRETTY_FUNCTION__<< " EXECUTING" << std::endl;
}


} // namespace toggl
