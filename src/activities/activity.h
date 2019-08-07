#ifndef SRC_ACTIVITY_H
#define SRC_ACTIVITY_H

namespace toggl {

class Context;

class Activity {
public:
    Activity(Context *context);
    virtual void work() = 0;

private:
    Context *context_ { nullptr };
};

};

#endif // SRC_ACTIVITY_H
