// Copyright 2020 Toggl Desktop developers.

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#include <Poco/Logger.h>
#include <string>
#include <utility>
#include <sstream>

namespace toggl {

/*
 * Beware, template black magic ahead
 *
 * Use Logger like this:
 *  Logger logger("myclass");
 *  logger.log("Running sync", foo, bar, "baz", 123);
 * and as long as all of the used types have a stringstream conversion, it'll run.
 * You can also declare your own stringstream << operator if you want to debug custom classes.
 */
class Logger {
public:
    Logger(const std::string &context)
        : context_(context)
    {}

    template <typename... Args>
    void trace(Args&&... args) const {
        std::stringstream ss;
        prepareOutput(ss, std::forward<Args>(args)...);
        Poco::Logger::get(context_).trace(ss.str());
    }
    template <typename... Args>
    void log(Args&&... args) const {
        std::stringstream ss;
        prepareOutput(ss, std::forward<Args>(args)...);
        Poco::Logger::get(context_).log(ss.str());
    }
    template <typename... Args>
    void debug(Args&&... args) const {
        std::stringstream ss;
        prepareOutput(ss, std::forward<Args>(args)...);
        Poco::Logger::get(context_).debug(ss.str());
    }
    template <typename... Args>
    void warning(Args&&... args) const {
        std::stringstream ss;
        prepareOutput(ss, std::forward<Args>(args)...);
        Poco::Logger::get(context_).warning(ss.str());
    }
    template <typename... Args>
    void error(Args&&... args) const {
        std::stringstream ss;
        prepareOutput(ss, std::forward<Args>(args)...);
        Poco::Logger::get(context_).error(ss.str());
    }

private:
    template <typename Arg>
    void prepareOutput(std::stringstream &ss, Arg&& arg) const {
        ss << std::forward<Arg>(arg);
    }
    template <typename Arg, typename... Args>
    void prepareOutput(std::stringstream &ss, Arg&& arg, Args&&... args) const {
        ss << std::forward<Arg>(arg);
        prepareOutput(ss, std::forward<Args>(args)...);
    }
private:
    std::string context_;
};

} // namespace toggl

#endif // SRC_LOGGER_H_
