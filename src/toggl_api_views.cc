// Copyright 2019 Toggl Desktop developers.

#include "./toggl_api.h"
#include "./toggl_api_views.h"
#include "./gui.h"

#include <string>

//template <typename To, class From> To convert(From * const f);
template <typename To, typename From> To convert(const From &f);
template <> const char_t *convert(const std::string &s) { return s.c_str(); }
template <> int convert(const int &i) { return i; }
template <> long convert(const long &l) { return l; }
template <> unsigned long convert(const unsigned long &ul) { return ul; }
template <> bool_t convert(const bool &b) { return b; }

//template <> const TogglGenericView *convert<const TogglGenericView*, toggl::view::Generic*>(const toggl::view::Generic *&v) { return nullptr; }
//template<> void* const* convert(toggl::view::Generic* const& v) {}

// has to be defined before including the definitions to expand the right macros
#define VIEWS_IMPLEMENTATION 1
#include "./toggl_api_def.h"
