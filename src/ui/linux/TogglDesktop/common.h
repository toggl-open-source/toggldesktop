// Copyright 2019 Toggl Desktop developers.

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#define PROPERTY(type, name) \
private: \
    Q_PROPERTY(type name READ name ## Get CONSTANT) \
public: \
    type name; \
    type name ## Get() const { return name; }

#endif // SRC_COMMON_H_
