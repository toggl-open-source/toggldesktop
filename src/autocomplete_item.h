// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_AUTOCOMPLETE_ITEM_H_
#define SRC_AUTOCOMPLETE_ITEM_H_

#include <string>
#include <sstream>

#include "./const.h"

#include "Poco/Types.h"

namespace toggl {

class AutocompleteItem {
 public:
    AutocompleteItem()
        : Text("")
    , Description("")
    , ProjectAndTaskLabel("")
    , TaskLabel("")
    , ProjectLabel("")
    , ClientLabel("")
    , ProjectColor("")
    , TaskID(0)
    , ProjectID(0)
    , Type(0)
    , WorkspaceName("") {}
    ~AutocompleteItem() {}

    bool IsTimeEntry() const {
        return kAutocompleteItemTE == Type;
    }
    bool IsTask() const {
        return kAutocompleteItemTask == Type;
    }
    bool IsProject() const {
        return kAutocompleteItemProject == Type;
    }
    bool IsWorkspace() const {
        return kAutocompleteItemWorkspace == Type;
    }

    std::string String() const {
        std::stringstream ss;
        ss << "AutocompleteItem"
           << " Text=" << Text
           << " Description=" << Description
           << " ProjectAndTaskLabel=" << ProjectAndTaskLabel
           << " TaskLabel=" << TaskLabel
           << " ProjectLabel=" << ProjectLabel
           << " ClientLabel=" << ClientLabel
           << " ProjectColor=" << ProjectColor
           << " TaskID=" << TaskID
           << " ProjectID=" << ProjectID
           << " Type=" << Type
           << " WorkspaceName=" << WorkspaceName;
        return ss.str();
    }

    std::string Text;
    std::string Description;
    std::string ProjectAndTaskLabel;
    std::string TaskLabel;
    std::string ProjectLabel;
    std::string ClientLabel;
    std::string ProjectColor;
    Poco::UInt64 TaskID;
    Poco::UInt64 ProjectID;
    Poco::UInt64 Type;
    std::string WorkspaceName;
};

}  // namespace toggl

#endif  // SRC_AUTOCOMPLETE_ITEM_H_
