// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_AUTOCOMPLETE_ITEM_H_
#define SRC_AUTOCOMPLETE_ITEM_H_

#include <string>
#include <sstream>

#include "./const.h"

#include "Poco/Types.h"

namespace kopsik {

class AutocompleteItem {
 public:
    AutocompleteItem()
        : Text("")
    , Description("")
    , ProjectAndTaskLabel("")
    , ProjectLabel("")
    , ClientLabel("")
    , ProjectColor("")
    , TaskID(0)
    , ProjectID(0)
    , Type(0) {}
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

	std::string String() const {
		std::stringstream ss;
		ss << "AutocompleteItem"
			<< " Text=" << Text
			<< " Description=" << Description
			<< " ProjectAndTaskLabel=" << ProjectAndTaskLabel
			<< " ProjectLabel=" << ProjectLabel
			<< " ClientLabel=" << ClientLabel
			<< " ProjectColor=" << ProjectColor
			<< " TaskID=" << TaskID
			<< " ProjectID=" << ProjectID
			<< " Type=" << Type;
		return ss.str();
	}

    std::string Text;
    std::string Description;
    std::string ProjectAndTaskLabel;
    std::string ProjectLabel;
    std::string ClientLabel;
    std::string ProjectColor;
    Poco::UInt64 TaskID;
    Poco::UInt64 ProjectID;
    Poco::UInt64 Type;
};

bool CompareAutocompleteItems(AutocompleteItem a, AutocompleteItem b);

}  // namespace kopsik

#endif  // SRC_AUTOCOMPLETE_ITEM_H_
