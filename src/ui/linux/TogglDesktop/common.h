// Copyright 2019 Toggl Desktop developers.

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <QMutex>

#define PROPERTY(type, name, ...) \
private: Q_PROPERTY(type name READ name##Get WRITE name##Set NOTIFY name##Changed) \
public: type name { __VA_ARGS__ }; \
        type name##Get() const { \
            propertyMutex_.lock(); \
            auto value = name; \
            propertyMutex_.unlock(); \
            return value; \
        } \
        Q_SLOT Q_INVOKABLE void name##Set(const type &val) { \
            propertyMutex_.lock(); \
            if (name != val) { \
                name = val; \
                propertyMutex_.unlock(); \
                emit name##Changed(); \
            } \
            else \
                propertyMutex_.unlock(); \
        } \
        Q_SIGNAL void name##Changed();

#endif // SRC_COMMON_H_
