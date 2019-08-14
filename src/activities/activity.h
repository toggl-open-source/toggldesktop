#ifndef SRC_ACTIVITY_H
#define SRC_ACTIVITY_H

#include <Poco/Activity.h>

namespace toggl {

class Context;
class ActivityManager;

class Activity {
public:
    Activity(ActivityManager *parent);

    Context *context();

    virtual void work() = 0;
    virtual void stop();
    virtual void start();
    virtual void restart();

protected:
    Poco::Activity<Activity> activity_;

private:
    ActivityManager *manager_;
};

};

#endif // SRC_ACTIVITY_H
