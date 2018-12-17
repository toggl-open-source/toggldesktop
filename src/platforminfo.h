#ifndef PLATFORMINFO_H
#define PLATFORMINFO_H

#include <sstream>

#if defined(__linux)

void TrackOsDetails(std::stringstream &ss);

#elif defined(_WIN32) || defined(WIN32)
inline void TrackOsDetails(std::stringstream &ss) {

}
#elif defined(__APPLE__)
inline void TrackOsDetails(std::stringstream &ss) {

}
#endif


#endif // PLATFORMINFO_H
