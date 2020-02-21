#ifndef SRC_MEMORY_H_
#define SRC_MEMORY_H_

#include "types.h"

#include <mutex>
#include <vector>
#include <iostream>
#include <map>

namespace toggl {
class BaseModel;
class RelatedData;

/**
 * @class locked
 * \brief A thread-safe wrapper class. Contains a unique_lock that gets unlocked when it goes out of scope
 *
 */
template <typename T>
class locked : public std::unique_lock<std::recursive_mutex> {
public:
    /**
     * @brief Constructs an empty invalid locked object
     */
    locked()
        : std::unique_lock<std::recursive_mutex>()
        , data_(nullptr)
    { }
    locked(locked<T> &&o) {
        std::unique_lock<std::recursive_mutex>::operator=(std::move(o));
        data_ = o.data_;
        o.data_ = nullptr;
    }
    /**
     * @brief Constructs a valid locked object
     * @param mutex - Needs to be a valid mutex, the application will block if it's already locked in a different thread
     * @param data - Pointer to data to protect
     */
    locked(mutex_type &mutex, T *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    { }
    locked<T> &operator=(locked<T> &&o) {
        std::unique_lock<std::recursive_mutex>::operator=(std::move(o));
        data_ = o.data_;
        o.data_ = nullptr;
        return *this;
    }
    ~locked() {}
    T *operator->() { return data_; }
    /**
     * @brief operator bool - Returns true if the mutex is locked and the pointer is not null
     */
    operator bool() const {
        return owns_lock() && data_;
    }
private:
    T *data_;
};


class ProtectedContainerBase {
public:
    ProtectedContainerBase(RelatedData *parent)
        : relatedData_(parent)
    {

    }
    virtual ~ProtectedContainerBase() {

    }

    RelatedData *GetRelatedData();
    const RelatedData *GetRelatedData() const;

    std::unique_lock<std::recursive_mutex> lock(bool immediately = true) {
        if (immediately)
            return std::unique_lock<std::recursive_mutex>(mutex_);
        return { mutex_, std::defer_lock };
    }
protected:
    RelatedData *relatedData_;
    mutable std::recursive_mutex mutex_;
};

template <class T>
class ProtectedModel : public ProtectedContainerBase {
public:
    ProtectedModel(RelatedData *parent)
        : ProtectedContainerBase(parent)
    {

    }
    void clear() {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        delete value_;
        value_ = nullptr;
    }
    template <typename ...Args>
    locked<T> create(Args&&... args) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        value_ = new T(this, std::forward<Args>(args)...);
        return { mutex_, value_ };
    }
    template<typename U> locked<U> make_locked(U *val) {
        return { mutex_, val };
    }
    template<typename U> locked<const U> make_locked(const U *val) const {
        return { mutex_, val };
    }
    locked<T> operator*() {
        return { mutex_, value_ };
    }
    locked<const T> operator*() const {
        return { mutex_, value_ };
    }
    locked<T> operator->() {
        return { mutex_, value_ };
    }
    locked<const T> operator->() const {
        return { mutex_, value_ };
    }
    operator bool() const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        return value_;
    }
private:
    T *value_;
};


/**
 * @class ProtectedContainer
 * \brief Contains convenience methods and a set of data to be protected with a single mutex.
 *
 * This class is intended to be used especially with BaseModel-based objects (TimeEntry, Project, etc.)
 * It also provides facilities to lock other objects using the internal mutex.
 */
template <class T>
class ProtectedContainer : public ProtectedContainerBase {
public:
    class iterator {
    public:
        friend class ProtectedContainer;
        typedef std::forward_iterator_tag iterator_category;

        iterator(ProtectedContainer *model, size_t position = SIZE_MAX)
            : lock(model->mutex_)
            , model(model)
            , position(position)
        { }
        iterator(const iterator &o)
            : lock(o.model->mutex_)
            , model(o.model)
            , position(o.position)
        { }
        ~iterator() {}

        iterator& operator=(const iterator &o) {
            lock = std::unique_lock<std::recursive_mutex>(o.model->mutex_);
            model = o.model;
            position = o.position;
            return *this;
        }
        bool operator==(const iterator &o) const {
            return model == o.model && realPosition() == o.realPosition();
        }
        bool operator!=(const iterator &o) const {
            return !(*this == o);
        }

        iterator& operator++() {
            position++;
            return *this;
        }
        locked<T> operator*() {
            if (realPosition() == SIZE_MAX)
                return {};
            return { model->mutex_, model->container_[position] };
        }
        T* operator->() const {
            if (realPosition() == SIZE_MAX)
                return {};
            return model->container_[position];
        }

    private:
        size_t realPosition() const {
            if (model->size() <= position)
                return SIZE_MAX;
            return position;
        }
        std::unique_lock<std::recursive_mutex> lock;
        ProtectedContainer *model;
        size_t position;
    };
    class const_iterator {
    public:
        friend class ProtectedContainer;
        typedef std::forward_iterator_tag iterator_category;

        const_iterator(const ProtectedContainer *model, size_t position = SIZE_MAX)
            : lock(model->mutex_)
            , model(model)
            , position(position)
        { }
        const_iterator(const const_iterator &o)
            : lock(o.model->mutex_)
            , model(o.model)
            , position(o.position)
        { }
        const_iterator(const iterator &o)
            : lock(o.model->mutex_)
            , model(o.model)
            , position(o.position)
        { }
        ~const_iterator() {}

        const_iterator& operator=(const const_iterator &o) {
            lock = std::unique_lock<std::recursive_mutex>(o.model->mutex_);
            model = o.model;
            position = o.position;
            return *this;
        }
        bool operator==(const const_iterator &o) const {
            return model == o.model && realPosition() == o.realPosition();
        }
        bool operator!=(const const_iterator &o) const {
            return !(*this == o);
        }

        const_iterator& operator++() {
            position++;
            return *this;
        }
        locked<const T> operator*() const {
            if (realPosition() == SIZE_MAX)
                return {};
            return { model->mutex_, model->container_[position]};
        }
        T* operator->() const {
            if (realPosition() == SIZE_MAX)
                return {};
            return model->container_[position];
        }

    private:
        size_t realPosition() const {
            if (model->size() <= position)
                return SIZE_MAX;
            return position;
        }
        std::unique_lock<std::recursive_mutex> lock;
        const ProtectedContainer *model;
        size_t position;
    };

    friend class iterator;
    friend class const_iterator;

    ProtectedContainer(RelatedData *parent)
        : ProtectedContainerBase(parent)
    {

    }
    ~ProtectedContainer() {
        clear();
    }

    iterator begin() { return iterator(this, 0); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator cbegin() const { return const_iterator(this, 0); }
    iterator end() { return iterator(this); }
    const_iterator end() const { return const_iterator(this); }
    const_iterator cend() const { return const_iterator(this); }

    iterator erase(iterator position) {
        if (position == end()) {
            // TODO warn?
            return end();
        }
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        T* ptr { nullptr };
        try {
            ptr = container_[position.position];
        }
        catch (std::out_of_range &) {
            // TODO warn?
            return end();
        }
        if (!ptr) {
            // again, warn?
            return end();
        }
        container_.erase(container_.begin() + position.position);
        guidMap_.erase(ptr->GUID());
        delete ptr;
        return position;
    }

    /**
     * @brief clear - Clear the @ref container_
     * @param deleteItems - Set to true if the pointers contained in the @ref container_ should be deleted, too
     */
    void clear() {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        for (auto i : container_)
            delete i;
        container_.clear();
        guidMap_.clear();
    }
    /**
     * @brief create - Allocate a new instance of <typename T>
     * @return - a @ref locked new instance of T
     * OVERHAUL TODO: this could be more efficient
     */
    template <typename ...Args>
    locked<T> create(Args&&... args) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        T *val = new T(this, std::forward<Args>(args)...);
        container_.push_back(val);
        guidMap_[val->GUID()] = val;
        return { mutex_, val };
    }
    /**
     * @brief remove - Remove one instance of <typename T>
     * @param guid - guid of the item to delete
     * @return - true if found and deleted
     */
    bool remove(const guid &guid) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        T* ptr { nullptr };
        try {
            ptr = guidMap_.at(guid);
        }
        catch (std::out_of_range &) {
            return false;
        }
        if (!ptr)
            return false;
        container_.erase(std::find(container_.begin(), container_.end(), ptr));
        guidMap_.erase(guid);
        delete ptr;
        return true;
    }
    /**
     * @brief size - Get how many items are contained inside
     * @return - number of items inside the container
     */
    size_t size() const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        return container_.size();
    }

    bool empty() const {
        return size() == 0;
    }

    bool contains(const guid &uuid) const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        return guidMap_.find(uuid) != guidMap_.end();
    }
    /**
     * @brief make_locked - Makes pointer to any type locked by the internal mutex
     * @param val - can be of any type (but a pointer)
     *
     * @warning beware of using pointers to non-static local variables with this method
     */
    template<typename U> locked<U> make_locked(U *val) const {
        return { mutex_, val };
    }
    /**
     * @brief make_locked - Makes pointer to any type locked by the internal mutex
     * @param val - can be of any type (but a pointer)
     *
     * @warning beware of using pointers to non-static local variables with this method
     */
    template<typename U> locked<const U> make_locked(const U *val) const {
        return { mutex_, val };
    }

    locked<T> operator[](size_t position) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        if (container_.size() > position)
            return { mutex_, container_[position] };
        return {};
    }
    locked<const T> operator[](size_t position) const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        if (container_.size() > position)
            return { mutex_, container_[position] };
        return {};
    }

    locked<T> operator[](const guid &uuid) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        try {
            return { mutex_, guidMap_.at(uuid) };
        }
        catch (std::out_of_range &) {
            return {};
        }
    }
    locked<const T> operator[](const guid &uuid) const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        try {
            return { mutex_, guidMap_.at(uuid) };
        }
        catch (std::out_of_range &) {
            return {};
        }
    }
    locked<T> byGUID(const guid &uuid) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        try {
            return { mutex_, guidMap_.at(uuid) };
        }
        catch (std::out_of_range &) {
            return {};
        }
    }
    locked<const T> byGUID(const guid &uuid) const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        try {
            return { mutex_, guidMap_.at(uuid) };
        }
        catch (std::out_of_range &) {
            return {};
        }
    }
    locked<T> byID(uint64_t id) {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        for (auto i : container_) {
            if (i->ID() == id)
                return { mutex_, i };
        }
        return {};
    }
    locked<const T> byID(uint64_t id) const {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        for (auto i : container_) {
            if (i->ID() == id)
                return { mutex_, i };
        }
        return {};
    }

    bool operator==(const ProtectedContainer &o) const { return this == &o; }
private:
    std::vector<T*> container_;
    std::map<guid, T*> guidMap_;
};

} // namespace toggl

#endif // SRC_MEMORY_H_
