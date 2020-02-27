#ifndef SRC_MEMORY_H_
#define SRC_MEMORY_H_

#include "../types.h"

#include <mutex>
#include <vector>
#include <iostream>
#include <map>
#include <tuple>

namespace toggl {
class BaseModel;
class RelatedData;
class Settings;
class TimelineEvent;

typedef std::recursive_mutex mutex_type;
typedef std::unique_lock<mutex_type> lock_type;

/**
 * @class locked
 * \brief A thread-safe wrapper class. Contains a unique_lock that gets unlocked when it goes out of scope
 *
 */
template <typename T>
class locked : public lock_type {
public:
    /**
     * @brief Constructs an empty invalid locked object
     */
    locked();
    /**
     * @brief locked move constructor
     * @param o other instance
     */
    locked(locked<T> &&o);
    /**
     * @brief Constructs a valid locked object
     * @param mutex - Needs to be a valid mutex, the application will block if it's already locked in a different thread
     * @param data - Pointer to data to protect
     */
    locked(mutex_type &mutex, T *data);
    /**
     * @brief operator = move assignment
     * @param o other instance
     * @return this moved instance
     */
    locked<T> &operator=(locked<T> &&o);
    ~locked() {}
    T *operator->();
    /** 
     * @brief operator bool - Returns true if the mutex is locked and the pointer is not null
     */
    operator bool() const;
    /*
     * Intentionally not implemented for most classes
     */
    T *operator*();

    // for polymorphism
    locked<BaseModel> base();
    // FIXME remove later
    locked<T> split();
private:
    T *data_ { nullptr };
};

/**
 * @brief The ProtectedBase class
 * @details Contains the commmon base for @ref ProtectedContainer and @ref ProtectedModel - it contains a mutex and a reference to a parent @ref RelatedData
 */
class ProtectedBase {
protected:
    /**
     * @brief Construct a ProtectedBase instance, not to be used directly
     * @param parent - a @ref RelatedData instsance
     */
    ProtectedBase(RelatedData *parent);
    ~ProtectedBase();
public:
    /**
     * @brief Get a pointer to the parent @ref RelatedData instance
     * @return Parent @ref RelatedData
     */
    RelatedData *GetRelatedData();
    /**
     * @brief Get a pointer to the parent @ref RelatedData instance
     * @return Parent @ref RelatedData
     */
    const RelatedData *GetRelatedData() const;

    lock_type lock(bool immediately = true);
    /**
     * @brief make_locked - Makes pointer to any type locked by the internal mutex
     * @param val - can be of any type (but a pointer)
     *
     * @warning beware of using pointers to non-static local variables with this method
     */
    template<typename U> locked<U> make_locked(U *val) const;
    /**
     * @brief make_locked - Makes pointer to any type locked by the internal mutex
     * @param val - can be of any type (but a pointer)
     *
     * @warning beware of using pointers to non-static local variables with this method
     */
    template<typename U> locked<const U> make_locked(const U *val) const;
protected:
    /**
     * @brief A static member to call a constructor of @ref T
     * @note This is here to prevent creation of @ref BaseModel instances directly outside Protected classes
     */
    template<typename T, typename... Args> static T *make(Args&&... args);

    RelatedData *relatedData_ { nullptr };
    mutable mutex_type mutex_;
};

/**
 * @brief The ProtectedModel class
 * @details This class is designed to keep a single instance of a @ref BaseModel based class and protect it with a mutex
 */
template <class T>
class ProtectedModel : public ProtectedBase {
public:
    /**
     * @brief ProtectedModel constructor
     * @param parent - a @ref RelatedData instance
     * @param allocate - if true, a default T constructor is called, otherwise stores a null value
     */
    ProtectedModel(RelatedData *parent, bool allocate = false);
    /**
     * @brief Delete a contained T instance and set the pointer to null
     */
    void clear();
    /**
     * @brief Construct a new instance of T - eventually delete a contained one if there is one
     */
    template <typename ...Args> locked<T> create(Args&&... args);
    /**
     * @brief operator* - Dereference
     * @return a @ref locked T instance
     */
    locked<T> operator*();
    /**
     * @brief operator* - Dereference
     * @return a @ref locked T instance
     */
    locked<const T> operator*() const;
    /**
     * @brief operator-> - Call a method on the contained object
     */
    locked<T> operator->();
    /**
     * @brief operator-> - Call a method on the contained object
     */
    locked<const T> operator->() const;
    /**
     * @brief operator bool
     * @return true if contains an object, false if null
     */
    operator bool() const;
protected:
    T *value_ { nullptr };
};

/**
 * @class ProtectedContainer
 * \brief Contains convenience methods and a set of data to be protected with a single mutex.
 *
 * This class is intended to be used especially with BaseModel-based objects (TimeEntry, Project, etc.)
 * It also provides facilities to lock other objects using the internal mutex.
 */
template <class T>
class ProtectedContainer : public ProtectedBase {
public:
    class iterator {
    public:
        friend class ProtectedContainer;
        typedef std::forward_iterator_tag iterator_category;

        iterator(ProtectedContainer *model, size_t position = SIZE_MAX);
        iterator(const iterator &o);
        ~iterator();

        iterator& operator=(const iterator &o);
        bool operator==(const iterator &o) const;
        bool operator!=(const iterator &o) const;

        iterator& operator++();
        locked<T> operator*();
        T* operator->() const;

    private:
        size_t realPosition() const;
        lock_type lock;
        ProtectedContainer *model;
        size_t position;
    };
    class const_iterator {
    public:
        friend class ProtectedContainer;
        typedef std::forward_iterator_tag iterator_category;

        const_iterator(const ProtectedContainer *model, size_t position = SIZE_MAX);
        const_iterator(const const_iterator &o);
        const_iterator(const iterator &o);
        ~const_iterator();

        const_iterator& operator=(const const_iterator &o);
        bool operator==(const const_iterator &o) const;
        bool operator!=(const const_iterator &o) const;

        const_iterator& operator++();
        locked<const T> operator*() const;
        T* operator->() const;

    private:
        size_t realPosition() const;
        lock_type lock;
        const ProtectedContainer *model;
        size_t position;
    };
    friend class iterator;
    friend class const_iterator;

    /**
     * @brief ProtectedContainer
     * @param parent
     */
    ProtectedContainer(RelatedData *parent);
    ProtectedContainer(const ProtectedContainer &o) = delete;
    ~ProtectedContainer();

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    iterator erase(iterator position);

    /**
     * @brief clear - Clear the @ref container_
     * @param deleteItems - Set to true if the pointers contained in the @ref container_ should be deleted, too
     */
    void clear();
    /**
     * @brief create - Allocate a new instance of <typename T>
     * @return - a @ref locked new instance of T
     * OVERHAUL TODO: this could be more efficient
     */
    template <typename ...Args>
    locked<T> create(Args&&... args);
    /**
     * @brief remove - Remove one instance of <typename T>
     * @param guid - guid of the item to delete
     * @return - true if found and deleted
     */
    bool remove(const guid &guid);
    /**
     * @brief size - Get how many items are contained inside
     * @return - number of items inside the container
     */
    size_t size() const;
    /**
     * @brief empty
     * @return true if @ref size == 0
     */
    bool empty() const;
    /**
     * @brief contains
     * @param uuid GUID/UUID of the element we're looking for
     * @return
     */
    bool contains(const guid &uuid) const;
    /**
     * @brief operator[] - access elements by their position
     * @param position - order in the underlying sequential container
     * @return locked instance of the element at the position
     * @warning This uses _position_, NOT IDs
     */
    locked<T> operator[](size_t position);
    locked<const T> operator[](size_t position) const;
    /**
     * @brief operator [] - access elements by their GUID/UUID
     * @param uuid - GUID/UUID of the element
     * @return locked instance of the element at the position (or an empty locked instance when not found)
     */
    locked<T> operator[](const guid &uuid);
    locked<const T> operator[](const guid &uuid) const;
    /**
     * @brief byGUID - access elements by their GUID/UUID
     * @param uuid - GUID/UUID of the element
     * @return locked instance of the element at the position (or an empty locked instance when not found)
     */
    locked<T> byGUID(const guid &uuid);
    locked<const T> byGUID(const guid &uuid) const;
    /**
     * @brief byID - access elements by their ID
     * @param uuid - ID of the element
     * @return locked instance of the element at the position (or an empty locked instance when not found)
     */
    locked<T> byID(uint64_t id);
    locked<const T> byID(uint64_t id) const;
    /**
     * @brief operator == - determine if two ProtectedContainers are same
     * @param o - other instance
     * @return true if the two instances are same
     */
    bool operator==(const ProtectedContainer &o) const;
private:
    std::vector<T*> container_;
    mutable std::map<guid, T*> guidMap_; // mutable because byGUID creates a search cache
};

// one argument variant, recursion stop
// returns a single unlocked lock
template<typename Arg>
std::tuple<lock_type> lockMoreImpl(Arg&& arg) {
    return std::make_tuple(arg.lock(false));
}

// recursively go through the list of protected containers
// return a tuple of unlocked locks
template<typename Arg, typename... Args>
auto lockMoreImpl(Arg&& arg, Args&&... args) {
    return std::tuple_cat(std::make_tuple(arg.lock(false)), lockMoreImpl(std::forward<Args>(args)...));
}

/**
 * @brief Lock more mutexes
 * This function locks all protected containers at once without causing a deadlock if the order is different in two places
 * Only accepted arguments are Protected* classes
 */
template<typename... Args>
std::vector<lock_type> lockMore(Args&&... args) {
    // Retrieve unlocked locks from all passed Protected instances
    // It is a tuple because only tuples can be used with std::apply
    auto tuple = lockMoreImpl(std::forward<Args>(args)...);
    // Lock them all at once
    std::apply([](auto&&... elems) {
        std::lock((elems)...);
    }, std::forward<decltype(tuple)>(tuple));
    // Transform the tuple to vector
    return std::apply([](auto&&... elems) {
        std::vector<lock_type> result;
        result.reserve(sizeof...(elems));
        (result.push_back(std::forward<decltype(elems)>(elems)), ...);
        return result;
    }, std::move(tuple));
}

/*******************************************************************************
 * IMPLEMENTATION
 */

//////// TEMPORARY /////////////////////////////////////////////////////////////
/*
 * TODO FIXME remove this eventually when GoogleAnalytics doesn't get passed an instance directly from another thread
 */
template<>
inline Settings *locked<Settings>::operator*() {
    return data_;
}
template<>
inline const Settings *locked<const Settings>::operator*() {
    return data_;
}
/*
 * TODO FIXME same for TimelineEvent
 */
template<>
inline TimelineEvent *locked<TimelineEvent>::operator*() {
    return data_;
}
template<>
inline const TimelineEvent *locked<const TimelineEvent>::operator*() {
    return data_;
}

//////// LOCKED ////////////////////////////////////////////////////////////////
template<typename T>
locked<BaseModel> locked<T>::base() {
    return locked<BaseModel>( *mutex(), data_ );
}

template<typename T>
locked<T> locked<T>::split() {
    return locked<T>( *mutex(), data_ );
}

template<typename T>
locked<T>::locked()
    : lock_type()
    , data_(nullptr)
{ }

template<typename T>
locked<T>::locked(locked<T> &&o) {
    lock_type::operator=(std::move(o));
    data_ = o.data_;
    o.data_ = nullptr;
}

template<typename T>
locked<T>::locked(mutex_type &mutex, T *data)
    : lock_type(mutex)
    , data_(data)
{ }

template<typename T>
locked<T> &locked<T>::operator=(locked<T> &&o) {
    lock_type::operator=(std::move(o));
    data_ = o.data_;
    o.data_ = nullptr;
    return *this;
}

template<typename T>
T *locked<T>::operator->() { return data_; }

template<typename T>
locked<T>::operator bool() const {
    return owns_lock() && data_;
}

//////// PROTECTEDBASE /////////////////////////////////////////////////////////
inline ProtectedBase::ProtectedBase(RelatedData *parent)
    : relatedData_(parent)
{

}

inline ProtectedBase::~ProtectedBase() {

}

inline RelatedData *ProtectedBase::GetRelatedData() {
    return relatedData_;
}

inline const RelatedData *ProtectedBase::GetRelatedData() const {
    return relatedData_;
}

inline lock_type ProtectedBase::lock(bool immediately) {
    if (immediately)
        return lock_type(mutex_);
    return { mutex_, std::defer_lock };
}

template<typename U>
locked<U> ProtectedBase::make_locked(U *val) const {
    return { mutex_, val };
}

template<typename U>
locked<const U> ProtectedBase::make_locked(const U *val) const {
    return { mutex_, val };
}

template<typename T, typename... Args>
T *ProtectedBase::make(Args&&... args) {
    return new T(args...);
}

//////// PROTECTEDMODEL ////////////////////////////////////////////////////////
template<class T>
ProtectedModel<T>::ProtectedModel(RelatedData *parent, bool allocate)
    : ProtectedBase(parent)
    , value_(allocate ? make<T>(this) : nullptr)
{

}

template<class T>
void ProtectedModel<T>::clear() {
    lock_type lock(mutex_);
    delete value_;
    value_ = nullptr;
}

template<class T>
locked<T> ProtectedModel<T>::operator*() {
    return { mutex_, value_ };
}

template<class T>
locked<const T> ProtectedModel<T>::operator*() const {
    return { mutex_, value_ };
}

template<class T>
locked<T> ProtectedModel<T>::operator->() {
    return { mutex_, value_ };
}

template<class T>
locked<const T> ProtectedModel<T>::operator->() const {
    return { mutex_, value_ };
}

template<class T>
toggl::ProtectedModel<T>::operator bool() const {
    lock_type lock(mutex_);
    return value_;
}

template <typename T> template <typename ...Args>
locked<T> ProtectedModel<T>::create(Args&&... args) {
    lock_type lock(mutex_);
    if (value_)
        delete value_;
    value_ = make<T>(this, std::forward<Args>(args)...);
    return { mutex_, value_ };
}

//////// PROTECTEDCONTAINER ////////////////////////////////////////////////////
//// base class
template<class T>
ProtectedContainer<T>::ProtectedContainer(RelatedData *parent)
    : ProtectedBase(parent)
{

}

template<class T>
ProtectedContainer<T>::~ProtectedContainer() {
    clear();
}

template<class T>
typename ProtectedContainer<T>::iterator ProtectedContainer<T>::begin() { return iterator(this, 0); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::begin() const { return const_iterator(this, 0); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::cbegin() const { return const_iterator(this, 0); }

template<class T>
typename ProtectedContainer<T>::iterator ProtectedContainer<T>::end() { return iterator(this); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::end() const { return const_iterator(this); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::cend() const { return const_iterator(this); }

template<class T>
typename ProtectedContainer<T>::iterator ProtectedContainer<T>::erase(ProtectedContainer<T>::iterator position) {
    if (position == end()) {
        // TODO warn?
        return end();
    }
    lock_type lock(mutex_);
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

template<class T>
void ProtectedContainer<T>::clear() {
    lock_type lock(mutex_);
    for (auto i : container_)
        delete i;
    container_.clear();
    guidMap_.clear();
}

template<class T>
bool ProtectedContainer<T>::remove(const guid &guid) {
    lock_type lock(mutex_);
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

template<class T>
size_t ProtectedContainer<T>::size() const {
    lock_type lock(mutex_);
    return container_.size();
}

template<class T>
bool ProtectedContainer<T>::empty() const {
    return size() == 0;
}

template<class T>
bool ProtectedContainer<T>::contains(const guid &uuid) const {
    lock_type lock(mutex_);
    return guidMap_.find(uuid) != guidMap_.end();
}

template<class T>
locked<T> ProtectedContainer<T>::operator[](size_t position) {
    lock_type lock(mutex_);
    if (container_.size() > position)
        return { mutex_, container_[position] };
    return {};
}

template<class T>
locked<const T> ProtectedContainer<T>::operator[](size_t position) const {
    lock_type lock(mutex_);
    if (container_.size() > position)
        return { mutex_, container_[position] };
    return {};
}

template<class T>
locked<T> ProtectedContainer<T>::operator[](const guid &uuid) {
    return byGUID(uuid);
}

template<class T>
locked<const T> ProtectedContainer<T>::operator[](const guid &uuid) const {
    return byGUID(uuid);
}

template<class T>
locked<T> ProtectedContainer<T>::byGUID(const guid &uuid) {
    lock_type lock(mutex_);
    // try checking the GUID cache
    try {
        auto ptr = guidMap_.at(uuid);
        // if the object has a different GUID
        if (ptr->GUID() != uuid) {
            // move it where it belongs
            guidMap_.erase(guidMap_.find(uuid));
            guidMap_.insert({uuid, ptr});
            // and check if there is any other object in the main container with the sought-after GUID
            for (auto i : container_) {
                if (i->GUID() == uuid) {
                    guidMap_.insert({uuid, i});
                    return { mutex_, ptr };
                }
            }
            return {};
        }
        return { mutex_, ptr };
    }
    catch (std::out_of_range &) {
        // if not found...
        for (auto i : container_) {
            // look into the main container if we have the GUID in question somewhere
            if (i->GUID() == uuid) {
                // and if we find it, look if this particular element was stored under a different GUID
                for (auto it = guidMap_.begin(); it != guidMap_.end(); ) {
                    // and if it was, delete it from the old location(s)
                    if (it->second == i)
                        it = guidMap_.erase(it);
                    else
                        ++it;
                }
                // then insert it to a location with the new GUID
                guidMap_.insert({uuid, i});
                // and return
                return { mutex_, i };
            }
        }
        return {};
    }
}

template<class T>
locked<const T> ProtectedContainer<T>::byGUID(const guid &uuid) const {
    lock_type lock(mutex_);
    // try checking the GUID cache
    try {
        auto ptr = guidMap_.at(uuid);
        // if the object has a different GUID
        if (ptr->GUID() != uuid) {
            // move it where it belongs
            guidMap_.erase(guidMap_.find(uuid));
            guidMap_.insert({uuid, ptr});
            // and check if there is any other object in the main container with the sought-after GUID
            for (auto i : container_) {
                if (i->GUID() == uuid) {
                    guidMap_.insert({uuid, i});
                    return { mutex_, ptr };
                }
            }
            return {};
        }
        return { mutex_, ptr };
    }
    catch (std::out_of_range &) {
        // if not found...
        for (auto i : container_) {
            // look into the main container if we have the GUID in question somewhere
            if (i->GUID() == uuid) {
                // and if we find it, look if this particular element was stored under a different GUID
                for (auto it = guidMap_.begin(); it != guidMap_.end(); ) {
                    // and if it was, delete it from the old location(s)
                    if (it->second == i)
                        it = guidMap_.erase(it);
                    else
                        ++it;
                }
                // then insert it to a location with the new GUID
                guidMap_.insert({uuid, i});
                // and return
                return { mutex_, i };
            }
        }
        return {};
    }
}

template<class T>
locked<T> ProtectedContainer<T>::byID(uint64_t id) {
    lock_type lock(mutex_);
    for (auto i : container_) {
        if (i->ID() == id)
            return { mutex_, i };
    }
    return {};
}

template<class T>
locked<const T> ProtectedContainer<T>::byID(uint64_t id) const {
    lock_type lock(mutex_);
    for (auto i : container_) {
        if (i->ID() == id)
            return { mutex_, i };
    }
    return {};
}

template<class T>
bool ProtectedContainer<T>::operator==(const ProtectedContainer<T> &o) const {
    return this == &o;
}

template <typename T> template <typename ...Args>
locked<T> ProtectedContainer<T>::create(Args&&... args) {
    lock_type lock(mutex_);
    T *val = make<T>(this, std::forward<Args>(args)...);
    container_.push_back(val);
    guidMap_[val->GUID()] = val;
    return { mutex_, val };
}


//// iterator ////////
template<class T>
ProtectedContainer<T>::iterator::iterator(ProtectedContainer *model, size_t position)
    : lock(model->mutex_)
    , model(model)
    , position(position)
{ }

template<class T>
ProtectedContainer<T>::iterator::iterator(const iterator &o)
    : lock(o.model->mutex_)
    , model(o.model)
    , position(o.position)
{ }

template<class T>
ProtectedContainer<T>::iterator::~iterator()
{ }

template<class T>
typename ProtectedContainer<T>::iterator &ProtectedContainer<T>::iterator::operator=(const iterator &o) {
    lock = lock_type(o.model->mutex_);
    model = o.model;
    position = o.position;
    return *this;
}

template<class T>
bool ProtectedContainer<T>::iterator::operator==(const ProtectedContainer<T>::iterator &o) const {
    return model == o.model && realPosition() == o.realPosition();
}

template<class T>
bool ProtectedContainer<T>::iterator::operator!=(const ProtectedContainer<T>::iterator &o) const {
    return !(*this == o);
}

template<class T>
typename ProtectedContainer<T>::iterator &ProtectedContainer<T>::iterator::operator++() {
    position++;
    return *this;
}

template<class T>
locked<T> ProtectedContainer<T>::iterator::operator*() {
    if (realPosition() == SIZE_MAX)
        return {};
    return { model->mutex_, model->container_[position] };
}

template<class T>
T *ProtectedContainer<T>::iterator::operator->() const {
    if (realPosition() == SIZE_MAX)
        return {};
    return model->container_[position];
}

template<class T>
size_t ProtectedContainer<T>::iterator::realPosition() const {
    if (model->size() <= position)
        return SIZE_MAX;
    return position;
}

//// const_iterator ////////
template<class T>
ProtectedContainer<T>::const_iterator::const_iterator(const ProtectedContainer *model, size_t position)
    : lock(model->mutex_)
    , model(model)
    , position(position)
{ }

template<class T>
ProtectedContainer<T>::const_iterator::const_iterator(const const_iterator &o)
    : lock(o.model->mutex_)
    , model(o.model)
    , position(o.position)
{ }

template<class T>
ProtectedContainer<T>::const_iterator::const_iterator(const iterator &o)
    : lock(o.model->mutex_)
    , model(o.model)
    , position(o.position)
{ }

template<class T>
ProtectedContainer<T>::const_iterator::~const_iterator()
{ }

template<class T>
typename ProtectedContainer<T>::const_iterator &ProtectedContainer<T>::const_iterator::operator=(const const_iterator &o) {
    lock = lock_type(o.model->mutex_);
    model = o.model;
    position = o.position;
    return *this;
}

template<class T>
bool ProtectedContainer<T>::const_iterator::operator==(const const_iterator &o) const {
    return model == o.model && realPosition() == o.realPosition();
}

template<class T>
bool ProtectedContainer<T>::const_iterator::operator!=(const const_iterator &o) const {
    return !(*this == o);
}

template<class T>
typename ProtectedContainer<T>::const_iterator &ProtectedContainer<T>::const_iterator::operator++() {
    position++;
    return *this;
}

template<class T>
locked<const T> ProtectedContainer<T>::const_iterator::operator*() const {
    if (realPosition() == SIZE_MAX)
        return {};
    return { model->mutex_, model->container_[position]};
}

template<class T>
T *ProtectedContainer<T>::const_iterator::operator->() const {
    if (realPosition() == SIZE_MAX)
        return {};
    return model->container_[position];
}

template<class T>
size_t ProtectedContainer<T>::const_iterator::realPosition() const {
    if (model->size() <= position)
        return SIZE_MAX;
    return position;
}


} // namespace toggl

#endif // SRC_MEMORY_H_
