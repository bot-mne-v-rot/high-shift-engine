#ifndef HIGH_SHIFT_HANDLE_MANAGER_H
#define HIGH_SHIFT_HANDLE_MANAGER_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>
#include <iostream>

template<typename T>
union Handle {
    struct {
        uint32_t index;
        uint32_t version;
    };
    uint64_t raw;
};

template<typename T>
class HandleManager {
public:
    HandleManager() = default;
    explicit HandleManager(std::size_t initial_cp);

    HandleManager(const HandleManager &other);
    HandleManager &operator=(const HandleManager &other);

    HandleManager(HandleManager &&other);
    HandleManager &operator=(HandleManager &&other);

    /**
     * Gets data associated with handle.
     * @return associated data if handle is valid, nullptr otherwise
     */
    [[nodiscard]] T *get(Handle<T> handle) const;

    /**
     * Creates new handle and associates it with data.
     * @return handle associated with data.
     */
    Handle<T> insert(T *data);

    /**
     * Associate new_data with handle.
     * @return true if handle is valid.
     */
    bool update(Handle<T> handle, T *new_data);

    /**
     * Invalidates handle.
     * @return ptr to associated data.
     * @return nullptr if handle was invalid.
     */
    T *erase(Handle<T> handle);

    bool is_valid(Handle<T> handle) const;

    void reserve(std::size_t new_capacity);

    std::size_t capacity() const;
    std::size_t size() const;

    void swap(HandleManager &other);

    ~HandleManager();

    /**
     * Iterates over each valid handle and calls functor with associated data.
     * @tparam Fn -- any functor with signature like void(T *)
     */
    template<typename U, typename Fn>
    friend void foreach(const HandleManager<U> &manager, Fn &&f);

private:
    struct Entry {
        T *data;
        uint32_t next;
        uint32_t version;
    };

    std::size_t sz = 0;
    std::size_t cp = 0;
    Entry *entries = nullptr;
    uint32_t free_list = NO_ENTRY;
    uint32_t used_list = NO_ENTRY;
    static const uint32_t NO_ENTRY = UINT32_MAX;
};

template<typename T>
HandleManager<T>::HandleManager(std::size_t initial_cp) {
    reserve(initial_cp);
}

template<typename T>
HandleManager<T>::HandleManager(const HandleManager &other) {
    *this = other;
}

template<typename T>
auto HandleManager<T>::operator=(const HandleManager &other) -> HandleManager & {
    if (this == &other)
        return *this;

    sz = other.sz;
    cp = other.cp;
    free_list = other.free_list;
    used_list = other.used_list;

    operator delete(entries);
    entries = (Entry *) operator new(cp * sizeof(Entry));
    memcpy(entries, other.entries, cp * sizeof(Entry));

    return *this;
}

template<typename T>
HandleManager<T>::HandleManager(HandleManager &&other) {
    swap(other);
}

template<typename T>
auto HandleManager<T>::operator=(HandleManager &&other) -> HandleManager & {
    swap(other);
    return *this;
}

template<typename T>
void HandleManager<T>::swap(HandleManager &other) {
    std::swap(sz, other.sz);
    std::swap(cp, other.cp);
    std::swap(entries, other.entries);
    std::swap(free_list, other.free_list);
    std::swap(used_list, other.used_list);
}

template<typename T>
void HandleManager<T>::reserve(std::size_t new_capacity) {
    if (new_capacity <= cp || new_capacity == 0)
        return;

    std::size_t new_cp = 1;
    while (new_cp < new_capacity)
        new_cp *= 2;

    Entry *new_entries = (Entry *) operator new(sizeof(Entry) * new_cp);
    if (entries)
        memcpy(new_entries, entries, cp * sizeof(Entry));
    for (std::size_t i = cp; i < new_cp; ++i) {
        new_entries[i].next = free_list;
        new_entries[i].version = 0;
        free_list = i;
    }

    cp = new_cp;
    operator delete(entries);
    entries = new_entries;
}

template<typename T>
HandleManager<T>::~HandleManager() {
    operator delete(entries);
}

template<typename T>
Handle<T> HandleManager<T>::insert(T *data) {
    if (free_list == NO_ENTRY)
        reserve(cp + 1);

    uint32_t free_idx = free_list;
    Entry &entry = entries[free_idx];

    free_list = entry.next;
    entry.data = data;

    entry.next = used_list;
    used_list = free_idx;

    Handle<T> handle;
    handle.index = free_idx;
    handle.version = entry.version;

    ++sz;

    return handle;
}

template<typename T>
T *HandleManager<T>::erase(Handle<T> handle) {
    if (handle.index >= cp)
        return nullptr;

    Entry &entry = entries[handle.index];
    if (entry.version != handle.version)
        return nullptr;

    T *data = entry.data;

    ++entry.version;

    used_list = entry.next;
    entry.next = free_list;
    free_list = handle.index;

    --sz;

    return data;
}

template<typename T>
bool HandleManager<T>::is_valid(Handle<T> handle) const {
    if (handle.index >= cp)
        return false;

    Entry &entry = entries[handle.index];
    if (entry.version != handle.version)
        return false;

    return true;
}

template<typename T>
bool HandleManager<T>::update(Handle<T> handle, T *new_data) {
    if (handle.index >= cp)
        return false;

    Entry &entry = entries[handle.index];
    if (entry.version != handle.version)
        return false;

    entry.data = new_data;

    return true;
}

template<typename T>
T *HandleManager<T>::get(Handle<T> handle) const {
    if (handle.index >= cp)
        return nullptr;

    Entry &entry = entries[handle.index];
    if (entry.version != handle.version)
        return nullptr;

    return entry.data;
}

template<typename T>
std::size_t HandleManager<T>::size() const {
    return sz;
}

template<typename T>
std::size_t HandleManager<T>::capacity() const {
    return cp;
}

template<typename T, typename Fn>
void foreach(const HandleManager<T> &manager, Fn &&f) {
    uint32_t cur = manager.used_list;
    while (cur != HandleManager<T>::NO_ENTRY) {
        auto &entry = manager.entries[cur];
        f(entry.data);
        cur = entry.next;
    }
}

#endif //HIGH_SHIFT_HANDLE_MANAGER_H
