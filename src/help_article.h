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
        const std::string &type,
        const std::string &name,
        const std::string &url,
        const std::string &search_text)
        : Type(type)
    , Name(name)
    , URL(url)
    , SearchText(search_text) {}

    virtual ~HelpArticle() {}

    std::string String() const {
        std::stringstream ss;
        ss  << "type=" << Type
            << " name=" << Name
            << " url=" << URL
            << " search text=" << SearchText;
        return ss.str();
    }

    std::string Type;
    std::string Name;
    std::string URL;
    std::string SearchText;
};

class HelpDatabase {
 public:
    HelpDatabase();
    virtual ~HelpDatabase() {}

    std::vector<HelpArticle> GetArticles(
        const std::string &keywords);

 private:
    std::vector<HelpArticle> articles_;
};

}  // namespace toggl

#endif  // SRC_HELP_ARTICLE_H_
