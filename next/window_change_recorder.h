// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WINDOW_CHANGE_RECORDER_H_
#define SRC_WINDOW_CHANGE_RECORDER_H_

#include <string>
#include <map>

#include "./timeline_notifications.h"
#include "./types.h"
#include "event_queue.h"

#include "Poco/Activity.h"

namespace Poco {
class Logger;
}

namespace toggl {

class WindowChangeRecorder : toggl::Event {
 public:
    explicit WindowChangeRecorder(TimelineDatasource *datasource, EventQueue *queue);
    ~WindowChangeRecorder();

    void SetIsLocked(bool isLocked);
    void SetIsSleeping(bool isSleeping);

    Poco::Mutex isSleeping_m_;
    bool isSleeping_;
protected:
   void requestSchedule(EventQueue *queue);
   void execute() override;

private:
   bool hasWindowChanged(const std::string &title, const std::string &filename) const;
   bool hasIdlenessChanged(const bool &idle) const;
   Poco::Logger &logger();
   bool getIsLocked();
   bool getIsSleeping();

   // Last window focus event data
   std::string last_title_;
   std::string last_filename_;
   time_t last_event_started_at_;
   bool last_idle_;

   TimelineDatasource *timeline_datasource_;

   std::string last_autotracker_title_;

   Poco::Mutex shutdown_m_;
   bool shutdown_;

   Poco::Mutex isLocked_m_;
   bool isLocked_;


    std::map<const int, int> timeline_errors_;
};

}  // namespace toggl

#endif  // SRC_WINDOW_CHANGE_RECORDER_H_
