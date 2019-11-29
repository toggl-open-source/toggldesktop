#ifndef PLATFORMINFO_H
#define PLATFORMINFO_H

#include <string>
#include <sstream>
#include <map>

#if defined(__linux)

void RetrieveOsDetails(std::stringstream &ss);
std::map<std::string, std::string> RetrieveOsDetailsMap();

#elif defined(_WIN32) || defined(WIN32)
inline void RetrieveOsDetails(std::stringstream &ss) {

}
inline std::map<std::string, std::string> RetrieveOsDetailsMap() {
    return {};
}
#elif defined(__APPLE__)
inline void RetrieveOsDetails(std::stringstream &ss) {

}
inline std::map<std::string, std::string> RetrieveOsDetailsMap() {
    return {};
}
#endif


#endif // PLATFORMINFO_H
