#ifndef ONLINE_TEST_MODEL_H
#define ONLINE_TEST_MODEL_H

#include "../toggl_api.h"

#include <string>
#include <list>

// a helper macro that will spit out the file, line and method to stderr when reached
#define WHEREAMI // do {std::cerr << __FILE__ << ":" << __LINE__ << "\t- " << __PRETTY_FUNCTION__ << std::endl; std::cerr.flush(); } while(0)

namespace test {

// template method that will create a list of Model objects from the Views
// it relies on the existence of a Toggl<something>View* constructor in the corresponding Model class
template <typename TestType, typename LibType> inline std::list<TestType> listFromView(LibType view) {
    std::list<TestType> ret;
    while (view) {
        ret.emplace_back(TestType { view });
        view = static_cast<LibType>(view->Next);
    }
    return ret;
}
// a specialization for a stringlist (while not technically a Model class, it simplifies the code in the dispatcher)
template <> inline std::list<std::string> listFromView<std::string>(char **list) {
    std::list<std::string> ret;
    while (list && *list) {
        ret.push_back(std::string(*list));
    }
    return ret;
}
// same as above, but doesn't create a list
template <typename TestType, typename LibType>
inline TestType oneFromView(LibType view) {
    if (view) {
        return TestType { view };
    }
    return TestType {};
}

struct Model {
    Model(const std::string &name, uint64_t id = 0) : name_(name), id_(id) {}
    std::string name_;
    uint64_t id_ { 0 };
    std::string guid_ {""};
};

struct Country : public Model {
    Country(const TogglCountryView *view = nullptr);
};

struct Workspace : public Model {
    Workspace(const TogglGenericView *view = nullptr);
};

struct Client : public Model {
    Client(const TogglGenericView *view = nullptr);
};

struct Project : public Model {
};

struct TimeEntry : public Model {
    TimeEntry(const TogglTimeEntryView *view = nullptr);
    uint64_t started_;
    std::string startedString_;
    uint64_t ended_;
    std::string endedString_;
    std::list<std::string> tags_;
    std::string projectLabel_;
    std::string clientLabel_;
    bool billable_;
};

struct Tag : public Model {
    Tag(const TogglGenericView *view = nullptr);
private:
};

struct Autocomplete : public Model {
    Autocomplete(const TogglAutocompleteView *view = nullptr);
private:
};

struct HelpArticle : public Model {
    HelpArticle(const TogglHelpArticleView *view = nullptr);
private:
};

struct Settings : public Model {
    Settings(const TogglSettingsView *view = nullptr);
    bool isNull() const;

    bool useProxy_ { false };
    std::string proxyHost_ { };
    uint64_t proxyPort_ { 0 };
    std::string proxyUsername_ { };
    std::string proxyPassword_ { };
    bool useIdleDetection_ { false };
    bool menubarTimer_ { false };
    bool menubarProject_ { false };
    bool dockIcon_ { false };
    bool onTop_ { false };
    bool reminder_ { false };
    bool recordTimeline_ { false };
    uint64_t idleMinutes_ { 0 };
    bool focusOnShortcut_ { false };
    uint64_t reminderMinutes_ { 0 };
    bool manualMode_ { false };
    bool analyticsOptedOut_ { false };
    bool autodetectProxy_ { false };
    bool remindMon_ { false };
    bool remindTue_ { false };
    bool remindWed_ { false };
    bool remindThu_ { false };
    bool remindFri_ { false };
    bool remindSat_ { false };
    bool remindSun_ { false };
    std::string remindStarts_ { };
    std::string remindEnds_ { };
    bool autotrack_ { false };
    bool openEditorOnShortcut_ { false };
    bool pomodoro_ { false };
    bool pomodoroBreak_ { false };
    uint64_t pomodoroMinutes_ { 0 };
    uint64_t pomodoroBreakMinutes_ { 0 };
    bool stopEntryOnShutdownSleep_ { false };
};

inline bool operator<(const test::Country &l, const test::Country &r) { return l.name_ < r.name_; }
inline bool operator<(const test::Workspace &l, const test::Workspace &r) { return l.name_ < r.name_; }
inline bool operator<(const test::Client &l, const test::Client &r) { return l.name_ < r.name_; }
inline bool operator<(const test::Project &l, const test::Project &r) { return l.name_ < r.name_; }
inline bool operator<(const test::TimeEntry &l, const test::TimeEntry &r) { return l.name_ < r.name_; }
inline bool operator<(const test::Tag &l, const test::Tag &r) { return l.name_ < r.name_; }
inline bool operator<(const test::Autocomplete &l, const test::Autocomplete &r) { return l.name_ < r.name_; }
inline bool operator<(const test::HelpArticle &l, const test::HelpArticle &r) { return l.name_ < r.name_; }

} // namespace test

#endif // ONLINE_TEST_MODEL_H
