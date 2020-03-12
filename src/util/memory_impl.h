#ifndef SRC_MEMORY_IMPL_H_
#define SRC_MEMORY_IMPL_H_

#include "memory.h" // technically circular but not a problem and will make syntax highlighting work right

namespace toggl {

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
template<class T>
ProtectedContainer<T>::ProtectedContainer(RelatedData *parent, comparison_function comparison)
    : ProtectedBase(parent)
    , container_(comparison)
{

}

template<class T>
ProtectedContainer<T>::~ProtectedContainer() {
    clear();
}

template<class T>
typename ProtectedContainer<T>::iterator ProtectedContainer<T>::begin() { return iterator(this, container_.begin()); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::begin() const { return const_iterator(this, container_.begin()); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::cbegin() const { return const_iterator(this, container_.cbegin()); }

template<class T>
typename ProtectedContainer<T>::iterator ProtectedContainer<T>::end() { return iterator(this, container_.end()); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::end() const { return const_iterator(this, container_.end()); }

template<class T>
typename ProtectedContainer<T>::const_iterator ProtectedContainer<T>::cend() const { return const_iterator(this, container_.cend()); }

template<class T>
typename ProtectedContainer<T>::iterator ProtectedContainer<T>::erase(ProtectedContainer<T>::iterator it) {
    if (it.it == container_.end()) {
        return end();
    }
    lock_type lock(mutex_);

    // checked if it's not end() at the start
    T* ptr = *(it.it);

    if (!ptr) {
        return end();
    }

    it.it = container_.erase(it.it);
    guidMap_.erase(ptr->GUID());
    delete ptr;
    return it;
}

template<class T>
void ProtectedContainer<T>::clear() {
    lock_type lock(mutex_);
    for (auto it = container_.begin(); it != container_.end(); ) {
        delete *it;
        it = container_.erase(it);
    }
    // should be empty by now anyway
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
bool ProtectedContainer<T>::shift(void *baseItem) {
    lock_type lock(mutex_);
    auto item = reinterpret_cast<T*>(baseItem);
    auto it = std::find(container_.begin(), container_.end(), item);
    if (it != container_.end()) {
        it = container_.erase(it);
        container_.insert(item);
        return true;
    }
    return false;
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
    if (container_.size() > position) {
        auto it = container_.begin();
        std::advance(it, position);
        return { mutex_, *it };
    }
    return {};
}

template<class T>
locked<const T> ProtectedContainer<T>::operator[](size_t position) const {
    lock_type lock(mutex_);
    if (container_.size() > position) {
        auto it = container_.cbegin();
        std::advance(it, position);
        return { mutex_, *it };
    }
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
    container_.insert(val);
    guidMap_[val->GUID()] = val;
    return { mutex_, val };
}


//// iterator ////////
template<class T>
ProtectedContainer<T>::iterator::iterator(ProtectedContainer *parent, typename container_type::iterator it)
    : lock(parent->mutex_)
    , parent(parent)
    , it(it)
{ }

template<class T>
ProtectedContainer<T>::iterator::iterator(const iterator &o)
    : lock(o.parent->mutex_)
    , parent(o.parent)
    , it(o.it)
{ }

template<class T>
ProtectedContainer<T>::iterator::~iterator()
{ }

template<class T>
typename ProtectedContainer<T>::iterator &ProtectedContainer<T>::iterator::operator=(const iterator &o) {
    lock = lock_type(o.parent->mutex_);
    parent = o.parent;
    it = o.it;
    return *this;
}

template<class T>
bool ProtectedContainer<T>::iterator::operator==(const ProtectedContainer<T>::iterator &o) const {
    return parent == o.parent && it == o.it;
}

template<class T>
bool ProtectedContainer<T>::iterator::operator!=(const ProtectedContainer<T>::iterator &o) const {
    return !(*this == o);
}

template<class T>
typename ProtectedContainer<T>::iterator &ProtectedContainer<T>::iterator::operator++() {
    it++;
    return *this;
}

template<class T>
locked<T> ProtectedContainer<T>::iterator::operator*() {
    if (it == parent->container_.end())
        return {};
    return { parent->mutex_, *it };
}

template<class T>
T *ProtectedContainer<T>::iterator::operator->() const {
    if (it == parent->container_.end())
        return nullptr;
    return *it;
}

//// const_iterator ////////
template<class T>
ProtectedContainer<T>::const_iterator::const_iterator(const ProtectedContainer *parent, typename container_type::const_iterator it)
    : lock(parent->mutex_)
    , parent(parent)
    , it(it)
{ }

template<class T>
ProtectedContainer<T>::const_iterator::const_iterator(const const_iterator &o)
    : lock(o.model->mutex_)
    , parent(o.parent)
    , it(o.it)
{ }

template<class T>
ProtectedContainer<T>::const_iterator::const_iterator(const iterator &o)
    : lock(o.model->mutex_)
    , parent(o.parent)
    , it(o.it)
{ }

template<class T>
ProtectedContainer<T>::const_iterator::~const_iterator()
{ }

template<class T>
typename ProtectedContainer<T>::const_iterator &ProtectedContainer<T>::const_iterator::operator=(const const_iterator &o) {
    lock = lock_type(o.model->mutex_);
    parent = o.parent;
    it = o.it;
    return *this;
}

template<class T>
bool ProtectedContainer<T>::const_iterator::operator==(const const_iterator &o) const {
    return parent == o.parent && it == o.it;
}

template<class T>
bool ProtectedContainer<T>::const_iterator::operator!=(const const_iterator &o) const {
    return !(*this == o);
}

template<class T>
typename ProtectedContainer<T>::const_iterator &ProtectedContainer<T>::const_iterator::operator++() {
    it++;
    return *this;
}

template<class T>
locked<const T> ProtectedContainer<T>::const_iterator::operator*() const {
    if (it == parent->container_.end())
        return {};
    return { parent->mutex_, *it };
}

template<class T>
T *ProtectedContainer<T>::const_iterator::operator->() const {
    if (it == parent->container_.end())
        return nullptr;
    return *it;
}

//////// LOCKMORE //////////////////////////////////////////////////////////////
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

} // namespace toggl

#endif // SRC_MEMORY_IMPL_H_
