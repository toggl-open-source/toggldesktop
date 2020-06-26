// Copyright 2020 Toggl Desktop developers.

#ifndef PROPERTY_H
#define PROPERTY_H

#include <memory>
#include <string>

namespace toggl {

/**
 * Property protection class
 * Tracks last unsynced change
 */
template <class T>
class Property {
public:
    typedef T value_type;

    Property(const T& value) {
        current_ = value;
        previous_ = value;
    }
    Property(T&& value = T {}) {
        current_ = std::move(value);
        previous_ = current_;
    }
    // on some beautiful day in the future, we could also be able to delete the copy assignment operator
    // it's required for settings, obmexperiments and timeline for now
    // Property& operator=(const Property &o) = delete;
    /* Dirtiness implementation */
    // Property is dirty when the current and previous versions of it are different.
    //     I opted for comparing these two instead of a bool flag because most of the strings
    // we're working with are pretty short (<24 characters) so SSO will make comparing them very fast.
    bool IsDirty() const {
        return current_ != previous_;
    }
    void SetNotDirty() {
        previous_ = current_;
    }
    /* Setters */
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
    /* Setters for current values only (previous value stays) */
    void SetCurrent(const T& current) {
        current_ = current;
    }
    void SetCurrent(const T&& current) {
        current_ = std::move(current);
    }
    /* Setters for previous values (to enable loading from db) */
    void SetPrevious(const T& previous) {
        previous_ = previous;
    }
    void SetPrevious(const T&& previous) {
        previous_ = std::move(previous);
    }
    /* Insert will modify current and previous value at once */
    void Insert(const T& previous, const T& current) {
        current_ = current;
        previous_ = previous;
    }
    void Insert(const T&& previous, const T&& current) {
        current_ = std::move(current);
        previous_ = std::move(previous);
    }
    /* Data access operators */
    // Notice that references are returned only as const
    // Only the -> operator is allowed to modify data inside (but should probably be const as well in the future)
    T* operator ->() {
        return &current_;
    }
    const T* operator ->() const {
        return &current_;
    }
    const T& operator()() const {
        return current_;
    }
    // TODO remove? confusing syntax with conversion operator
    /*
    operator const T() const {
        return current_;
    }
    */
    const T& Get() const {
        return current_;
    }
    /* Equality */
    bool operator ==(const Property<T> &o) const {
        return current_ == o.current_;
    }
    bool operator ==(const T &o) const {
        return current_ == o;
    }
    bool operator !=(const Property<T> &o) const {
        return current_ != o.current_;
    }
    bool operator !=(const T &o) const {
        return current_ != o;
    }
private:
    T current_;
    T previous_;
};

/* A helper method to check if any of the selected properties is dirty */
template<typename... Args> static bool IsAnyPropertyDirty(Args&... args) { return (... || args.IsDirty()); }
/* A helper method to clear the dirty flag for more properties at once */
template<typename... Args> static void AllPropertiesClearDirty(Args&... args) { (... , args.SetNotDirty()); }

} // namespace toggl

#endif // PROPERTY_H
