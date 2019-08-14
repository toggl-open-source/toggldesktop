#ifndef SRC_ACTIVITY_H
#define SRC_ACTIVITY_H

#include <Poco/Activity.h>

namespace toggl {

class Context;

class Activity {
public:
    Activity(Context *context);
    virtual void work() = 0;
    virtual void start();

protected:
    Poco::Activity<Activity> activity_;
    Context *context_ { nullptr };
};

};

#endif // SRC_ACTIVITY_H
