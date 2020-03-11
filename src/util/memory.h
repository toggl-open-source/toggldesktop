#ifndef SRC_MEMORY_H_
#define SRC_MEMORY_H_

#include "../types.h"

#include <mutex>
#include <vector>
#include <set>
#include <iostream>
#include <map>
#include <tuple>
#include <functional>
#include <algorithm>

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

    // TODO figure out if it's possible to do without void*
    virtual bool shift(void *item) { return false; }

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
    typedef ProtectedContainer<T> type;
    typedef T value_type;
    typedef std::function<bool(const T*, const T*)> comparison_function;
    using container_type = std::set<T*, comparison_function>;

    class iterator {
    public:
        friend class ProtectedContainer;
        typedef std::forward_iterator_tag iterator_category;

        iterator(ProtectedContainer *parent, typename container_type::iterator it);
        iterator(const iterator &o);
        ~iterator();

        iterator& operator=(const iterator &o);
        bool operator==(const iterator &o) const;
        bool operator!=(const iterator &o) const;

        iterator& operator++();
        locked<T> operator*();
        T* operator->() const;

    private:
        lock_type lock;
        ProtectedContainer *parent;
        typename container_type::iterator it;
    };
    class const_iterator {
    public:
        friend class ProtectedContainer;
        typedef std::forward_iterator_tag iterator_category;

        const_iterator(const ProtectedContainer *parent, typename container_type::const_iterator it);
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
        lock_type lock;
        const ProtectedContainer *parent;
        typename container_type::const_iterator it;
    };
    friend class iterator;
    friend class const_iterator;

    /**
     * @brief ProtectedContainer
     * @param parent - the parent RelatedData instances (to be passed to the children)
     * @param comparison - a binary predicate with the signature of bool(const T*, const T*), used to insert items at the right position when creating
     * TODO Using the comparison predicate has O(N) complexity, we'd very likely be much better off storing everything in a std::set
     */
    ProtectedContainer(RelatedData *parent, std::function<bool(const T* left, const T* right)> comparison = [](const T* l, const T* r){ return l < r; });
    ProtectedContainer(const ProtectedContainer &o) = delete;
    ~ProtectedContainer();

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    iterator erase(iterator it);

    /**
     * @brief clear - Clear the @ref container_
     * @param deleteItems - Set to true if the pointers contained in the @ref container_ should be deleted, too
     */
    void clear();
    /**
     * @brief create - Allocate a new instance of <typename T>. If a comparison predicate was supplied, the item is inserted at the right order
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

    bool shift(void *item) override;
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
     * @warning This uses _position_, NOT IDs - with set it's very slow
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
    container_type container_;
    mutable std::map<guid, T*> guidMap_; // mutable because byGUID creates a search cache
};


/**
 * @brief Lock more mutexes
 * This function locks all protected containers at once without causing a deadlock if the order is different in two places
 * Only accepted arguments are Protected* classes
 */
template<typename... Args> std::vector<lock_type> lockMore(Args&&... args);

} // namespace toggl

#include "memory_impl.h"

#endif // SRC_MEMORY_H_
