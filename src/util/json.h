#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <Poco/Types.h>
#include <json/json.h>

template <class From> Json::Value JsonConvert(From);
template<> inline Json::Value JsonConvert(Poco::Int64 val) { return static_cast<Json::Int64>(val); }
template<> inline Json::Value JsonConvert(Poco::UInt64 val) { return static_cast<Json::UInt64>(val); }
template<> inline Json::Value JsonConvert(std::string val) { return val; }

#endif // JSONHELPER_H
