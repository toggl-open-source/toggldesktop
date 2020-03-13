#ifndef SRC_STRING_TOOLS_H_
#define SRC_STRING_TOOLS_H_

#include <string>
#include <sstream>

namespace toggl {

/**
 * @brief join - takes a container of <stuff> and merges it into a string, putting a delimiter between the items
 * Example:
 *   join(std::list<int>{1, 2, 3}, " - "); -> "1 - 2 - 3"
 *   join(std::list<std::string>{"foo", "bar", "baz"}, ", "); -> "foo, bar, baz"
 */
template <typename T>
std::string join(const T &v, const std::string &delim) {
    std::ostringstream s;
    bool first = true;
    for (const auto& i : v) {
        if (first) {
            first = false;
        }
        else {
            s << delim;
        }
        s << i;
    }
    return s.str();
}

} // namespace toggl

#endif // SRC_STRING_TOOLS_H_
