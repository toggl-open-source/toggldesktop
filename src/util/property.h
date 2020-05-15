// Copyright 2020 Toggl Desktop developers.

#ifndef PROPERTY_H
#define PROPERTY_H

#include <memory>

namespace toggl {

template <class T>
class Property {
public:
    Property(const T& value) {
        current_ = value;
        previous_ = value;
    }
    Property(T&& value) {
        current_ = std::move(value);
        previous_ = current_;
    }
    void Set(const T& value, bool makeDirty = true) {
        if (makeDirty)
            previous_ = std::move(current_);
        else
            previous_ = value;
        current_ = value;
    }
    void Set(const T&& value, bool makeDirty = true) {
        if (makeDirty)
            previous_ = std::move(current_);
        else
            previous_ = std::move(value);
        current_ = value;
    }
    T* operator ->() {
        return &current_;
    }
    const T* operator ->() const {
        return &current_;
    }
    const T& operator()() const {
        return current_;
    }
    operator const T() const {
        return current_;
    }
    const T& Get() const {
        return current_;
    }
    bool IsDirty() const {
        return current_ != previous_;
    }
    bool SetNotDirty() {
        previous_ = current_;
    }
private:
    T current_;
    T previous_;
};

template<typename... Args> static bool IsAnyPropertyDirty(Args&... args) { return (... || args.IsDirty()); }
template<typename... Args> static void MarkAllPropertiesNotDirty(Args&... args) { (... , args.SetNotDirty()); }


} // namespace toggl

#endif // PROPERTY_H
