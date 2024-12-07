
#ifndef JSTD_HASH_CHUNK_LIST_H
#define JSTD_HASH_CHUNK_LIST_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <assert.h>

#include <cstdint>
#include <cstddef>      // For std::ptrdiff_t, std::size_t
#include <list>
#include <vector>
#include <memory>       // For std::swap()
#include <type_traits>  // For std::forward<T>
#include <stdexcept>    // For std::out_of_range()

namespace jstd {

//
// hash_entry_chunk<T>
//
template <typename T>
struct JSTD_DLL hash_entry_chunk {
    typedef T               entry_type;
    typedef std::size_t     size_type;

    entry_type * entries;
    size_type    size;
    size_type    capacity;
    size_type    chunk_id;

    hash_entry_chunk() : entries(nullptr), size(0), capacity(0), chunk_id(0) {}
    hash_entry_chunk(entry_type * entries, size_type size,
                     size_type capacity, size_type chunk_id)
        : entries(entries), size(size), capacity(capacity), chunk_id(chunk_id) {}
    hash_entry_chunk(const hash_entry_chunk & src)
        : entries(src.entries), size(src.size),
          capacity(src.capacity), chunk_id(src.chunk_id) {}
    ~hash_entry_chunk() {}

    hash_entry_chunk & operator = (const hash_entry_chunk & rhs) {
        this->entries = rhs.entries;
        this->size = rhs.size;
        this->capacity = rhs.capacity;
        this->chunk_id = rhs.chunk_id;
        return *this;
    }

    size_type free_cnt() const { return size_type(this->capacity() - this->size()); }

    size_type is_empty() const { return (this->size == 0); }
    size_type is_full() const { return (this->size >= this->capacity); }

    void set_chunk(entry_type * entries, size_type size,
                   size_type capacity, size_type chunk_id) {
        this->entries = entries;
        this->size = size;
        this->capacity = capacity;
        this->chunk_id = chunk_id;
    }

    void set_entries(entry_type * entries) {
        this->entries = entries;
    }

    void set_size(size_type size) {
        this->size = size;
    }

    void set_capacity(size_type capacity) {
        this->capacity = capacity;
    }

    void set_chunk_id(size_type chunk_id) {
        this->chunk_id = chunk_id;
    }

    void clear() {
        this->entries = nullptr;
        this->size = 0;
        this->capacity = 0;
        this->chunk_id = 0;
    }

    void reset() {
        this->size = 0;
    }

    void increase() {
        ++(this->size);
    }

    void decrease() {
        assert(this->size > 0);
        --(this->size);
    }

    void expand(size_type size) {
        this->size += size;
    }

    void shrink(size_type size) {
        assert(this->size >= size);
        this->size -= size;
    }

    void swap(hash_entry_chunk<T> & other) {
        std::swap(this->entries,  other.entries);
        std::swap(this->size,     other.size);
        std::swap(this->capacity, other.capacity);
        std::swap(this->chunk_id, other.chunk_id);
    }
};

template <typename T>
inline void swap(hash_entry_chunk<T> & lhs, hash_entry_chunk<T> & rhs) {
    lhs.swap(rhs);
}

template <typename T, typename Allocator, typename EntryAllocator>
class JSTD_DLL hash_entry_chunk_list {
public:
    typedef T                                       entry_type;
    typedef hash_entry_chunk<T>                     entry_chunk_t;
    typedef hash_entry_chunk<T>                     element_type;

    typedef std::vector<entry_chunk_t>              vector_type;
    typedef typename vector_type::value_type        value_type;
    typedef typename vector_type::size_type         size_type;
    typedef typename vector_type::difference_type   difference_type;
    typedef typename vector_type::iterator          iterator;
    typedef typename vector_type::const_pointer     const_pointer;
    typedef typename vector_type::reference         reference;
    typedef typename vector_type::const_reference   const_reference;

    typedef Allocator                               allocator_type;
    typedef EntryAllocator                          entry_allocator_type;

    typedef hash_entry_chunk_list<T, Allocator, EntryAllocator>
                                                    this_type;

private:
    std::list<entry_chunk_t>    last_chunk_;
    std::vector<entry_chunk_t>  chunk_list_;
    allocator_type              allocator_;
    entry_allocator_type        entry_allocator_;

public:
    hash_entry_chunk_list() {}

    ~hash_entry_chunk_list() {
        this->destroy();
    }

    size_type size() const       { return this->chunk_list_.size();     }
    size_type capacity() const   { return this->chunk_list_.capacity(); }

    iterator begin()             { return this->chunk_list_.begin();  }
    iterator end()               { return this->chunk_list_.end();    }
    const_pointer begin() const  { return this->chunk_list_.begin();  }
    const_pointer end() const    { return this->chunk_list_.end();    }

    iterator cbegin()            { return this->chunk_list_.cbegin(); }
    iterator cend()              { return this->chunk_list_.cend();   }
    const_pointer cbegin() const { return this->chunk_list_.cbegin(); }
    const_pointer cend() const   { return this->chunk_list_.cend();   }

    reference front() { return this->chunk_list_.front(); }
    reference back()  { return this->chunk_list_.back();  }

    const_reference front() const { return this->chunk_list_.front(); }
    const_reference back() const  { return this->chunk_list_.back();  }

    bool is_empty() const { return (this->size() == 0); }
    bool is_full() const {
        return false;
    }

    bool hasLastChunks() const {
        return (this->last_chunk_.entries() != nullptr);
    }

    std::list<entry_chunk_t> & lastChunks() {
        return this->last_chunk_;
    }

    const std::list<entry_chunk_t> & lastChunks() const {
        return this->last_chunk_;
    }

    size_type lastChunkCount() const {
        return this->last_chunk_.size();
    }

    uint32_t lastChunkId() const {
        return static_cast<uint32_t>(this->last_chunk_[0].chunk_id);
    }

    size_type firstFreeIndex() const {
        return this->last_chunk_[0].size;
    }

    element_type & operator [] (size_type pos) {
        return this->chunk_list_[pos];
    }

    const element_type & operator [] (size_type pos) const {
        return this->chunk_list_[pos];
    }

    element_type & at(size_type pos) {
        if (pos < this->size())
            return this->chunk_list_[pos];
        else
            throw std::out_of_range("hash_entry_chunk_list<T>::at(pos) out of range.");
    }

    const element_type & at(size_type pos) const {
        if (pos < this->size())
            return this->chunk_list_[pos];
        else
            throw std::out_of_range("hash_entry_chunk_list<T>::at(pos) out of range.");
    }

    void destroy() {
        if (likely(this->chunk_list_.size() > 0)) {
            size_type last_index = this->chunk_list_.size() - 1;
            for (size_type i = 0; i < last_index; i++) {
                entry_type * entries = this->chunk_list_[i].entries;
                if (likely(entries != nullptr)) {
                    size_type capacity = this->chunk_list_[i].capacity;
                    entry_type * entry = entries;
                    assert(entry != nullptr);
                    for (size_type j = 0; j < capacity; j++) {
                        if (likely(entry->attrib.isInUseEntry() || entry->attrib.isReusableEntry())) {
                            this->allocator_.destroy(&entry->value);
                        }
                        ++entry;
                    }

                    // Free the entries buffer.
                    this->entry_allocator_.deallocate(entries, capacity);
                }
            }

            // Destroy last entry
            {
                entry_type * last_entries = this->last_chunk_.entries;
                if (likely(last_entries != nullptr)) {
                    size_type last_size     = this->last_chunk_.size;
                    size_type last_capacity = this->last_chunk_.capacity;
                    assert(last_size <= last_capacity);
                    entry_type * entry = last_entries;
                    assert(entry != nullptr);
                    for (size_type j = 0; j < last_size; j++) {
                        if (likely(entry->attrib.isInUseEntry() || entry->attrib.isReusableEntry())) {
                            this->allocator_.destroy(&entry->value);
                        }
                        ++entry;
                    }

                    // Free the entries buffer.
                    this->entry_allocator_.deallocate(last_entries, last_capacity);
                }
            }
        }

        this->clear();
    }

    void clear() {
        this->last_chunk_.clear();
        this->chunk_list_.clear();
    }

    void reserve(size_type count) {
        this->chunk_list_.reserve(count);
    }

    void resize(size_type new_size) {
        this->chunk_list_.resize(new_size);
    }

    void addChunk(const entry_chunk_t & chunk) {
        assert(this->is_full());

        size_type chunk_id = this->chunk_list_.size();
        this->last_chunk_.set_chunk(chunk.entries, chunk.size, chunk.capacity, chunk_id);
        this->chunk_list_.emplace_back(chunk);
    }

    void addChunk(entry_type * entries, size_type entry_size, size_type entry_capacity) {
        assert(this->is_full());

        size_type chunk_id = this->chunk_list_.size();
        this->last_chunk_.set_chunk(entries, entry_size, entry_capacity, chunk_id);
        this->chunk_list_.emplace_back(entries, entry_size, entry_capacity, chunk_id);
    }

    void removeLastChunk() {
        assert(!this->chunk_list_.empty());
        this->chunk_list_.pop_back();
    }

    void rebuildLastChunk() {
        if (this->size() > 1) {
            size_type last_chunk_id = this->chunk_list_.size() - 1;
            this->last_chunk_ = this->chunk_list_[last_chunk_id];
            this->last_chunk_.size = this->last_chunk_.capacity;
        }
        else if (this->size() == 0) {
            this->last_chunk_.clear();
        }
    }

    void appendEntry(uint32_t chunk_id) {
        assert(chunk_id < this->size());
        this->chunk_list_[chunk_id].increase();
    }

    void appendFreeEntry(entry_type * entry) {
        uint32_t chunk_id = entry->attrib.getChunkId();
        this->appendEntry(chunk_id);
    }

    void removeEntry(uint32_t chunk_id) {
        assert(chunk_id < this->size());
        this->chunk_list_[chunk_id].decrease();
    }

    void removeEntry(entry_type * entry) {
        uint32_t chunk_id = entry->attrib.getChunkId();
        this->removeEntry(chunk_id);
    }

    size_type findClosedChunk(size_type target_capacity) {
        size_type target_chunk_id = size_type(-1);
        size_type max_target_size = size_type(-1);
        size_type total_chunk_size = this->chunk_list_.size() - 1;
        for (size_type i = 0; i < total_chunk_size; i++) {
            if (likely(this->chunk_list_[i].entries != nullptr)) {
                size_type chunk_capacity = this->chunk_list_[i].capacity;
                if (likely(chunk_capacity == target_capacity)) {
                    size_type chunk_size = this->chunk_list_[i].size;
                    if (std::ptrdiff_t(chunk_size) > std::ptrdiff_t(max_target_size)) {
                        max_target_size = chunk_size;
                        target_chunk_id = i;
                    }
                }
                else if (likely(chunk_capacity > target_capacity)) {
                    break;
                }
            }
        }

        return target_chunk_id;
    }

    void swap(this_type & other) {
        if (&other != this) {
            this->last_chunk_.swap(other.last_chunk_);
            std::swap(this->chunk_list_, other.chunk_list_);
            if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
                std::swap(this->allocator_, other.allocator_);
            }
            if (std::allocator_traits<entry_allocator_type>::propagate_on_container_swap::value) {
                std::swap(this->entry_allocator_, other.entry_allocator_);
            }
        }
    }

    template <typename OtherAllocator, typename OtherEntryAllocator>
    void swap(hash_entry_chunk_list<T, OtherAllocator, OtherEntryAllocator> & other) {
        if (&other != this) {
            this->last_chunk_.swap(other.last_chunk_);
            std::swap(this->chunk_list_, other.chunk_list_);
            if (std::allocator_traits<OtherAllocator>::propagate_on_container_swap::value) {
                std::swap(this->allocator_, other.allocator_);
            }
            if (std::allocator_traits<OtherEntryAllocator>::propagate_on_container_swap::value) {
                std::swap(this->entry_allocator_, other.entry_allocator_);
            }
        }
    }
};

template <typename T, typename Allocator, typename EntryAllocator>
inline void swap(hash_entry_chunk_list<T, Allocator, EntryAllocator> & lhs,
                 hash_entry_chunk_list<T, Allocator, EntryAllocator> & rhs) {
    lhs.swap(rhs);
}

} // namespace jstd

#endif // JSTD_HASH_CHUNK_LIST_H
