// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_HELP_ARTICLE_H_
#define SRC_HELP_ARTICLE_H_

#include <string>
#include <sstream>

namespace toggl {

class HelpArticle {
 public:
    HelpArticle(
        const std::string cat,
        const std::string name,
        const std::string url)
        : Category(cat)
    , Name(name)
    , URL(url) {}

    virtual ~HelpArticle() {}

    std::string String() const {
        std::stringstream ss;
        ss  << "category=" << Category
            << " name=" << Name
            << " url=" << URL;
        return ss.str();
    }

    std::string Category;
    std::string Name;
    std::string URL;
};

}  // namespace toggl

#endif  // SRC_HELP_ARTICLE_H_
