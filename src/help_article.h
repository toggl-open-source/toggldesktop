// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_HELP_ARTICLE_H_
#define SRC_HELP_ARTICLE_H_

#include <sstream>
#include <string>
#include <vector>

namespace toggl {

class HelpArticle {
 public:
    HelpArticle(
        const std::string type,
        const std::string name,
        const std::string url)
        : Type(type)
    , Name(name)
    , URL(url) {}

    virtual ~HelpArticle() {}

    std::string String() const {
        std::stringstream ss;
        ss  << "type=" << Type
            << " name=" << Name
            << " url=" << URL;
        return ss.str();
    }

    static std::vector<HelpArticle> GetArticles(
        const std::string keywords);

    std::string Type;
    std::string Name;
    std::string URL;
};

}  // namespace toggl

#endif  // SRC_HELP_ARTICLE_H_
