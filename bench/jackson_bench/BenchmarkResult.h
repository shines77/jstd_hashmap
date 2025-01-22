
#ifndef JTEST_BENCHMARK_H
#define JTEST_BENCHMARK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"
#include "jstd/basic/inttypes.h"

#include <stdio.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <type_traits>

namespace jtest {

class BenchmarkBase {
public:
    using size_type = std::size_t;

protected:
    size_type   id_;
    std::string name_;
    std::string label_;

public:
    BenchmarkBase() : id_(size_type(-1)) {}
    BenchmarkBase(const std::string & name, const std::string & label = "")
        : id_(size_type(-1)), name_(name), label_(label) {}

    BenchmarkBase(const BenchmarkBase & src)
        : id_(src.id()),
          name_(src.name()), label_(src.label()) {}

    BenchmarkBase(BenchmarkBase && src)
        : id_(src.id()),
          name_(std::move(src.name())), label_(std::move(src.label())) {}

    ~BenchmarkBase() {}

    size_type id() const {
        return this->id_;
    }

    void setId(size_type id) {
        this->id_ = id;
    }

    std::string & name() {
        return this->name_;
    }

    const std::string & name() const {
        return this->name_;
    }

    std::string & label() {
        return this->label_;
    }

    const std::string & label() const {
        return this->label_;
    }

    void setName(const std::string & name) {
        this->name_ = name;
    }

    void setLabel(const std::string & label) {
        this->label_ = label;
    }

    void setName(const std::string & name, const std::string & label) {
        this->name_ = name;
        this->label_ = label;
    }

    void swap(BenchmarkBase & other) noexcept {
        if (std::addressof(other) != this) {
            std::swap(this->id_,    other.id_);
            std::swap(this->name_,  other.name_);
            std::swap(this->label_, other.label_);
        }
    }
};

namespace detail {

template <bool IsPointer, typename ValueT, typename ElementT,
          typename std::enable_if<IsPointer>::type * = nullptr,
          typename ... Args>
ValueT construct_if(Args && ... args)
{
    return new ElementT(std::forward<Args>(args)...);
}

template <bool IsPointer, typename ValueT, typename ElementT,
          typename std::enable_if<!IsPointer>::type * = nullptr,
          typename ... Args>
ValueT construct_if(Args && ... args)
{
    return std::move(ElementT(std::forward<Args>(args)...));
}

template <bool IsPointer, typename VectorT,
          typename std::enable_if<IsPointer>::type * = nullptr>
void destroy_if(VectorT & vector)
{
    using value_type = typename VectorT::value_type;
    for (std::size_t i = 0; i < vector.size(); i++) {
        value_type element = vector[i];
        if (element != nullptr) {
            delete element;
            vector[i] = nullptr;
        }
    }
    vector.clear();
}

template <bool IsPointer, typename VectorT,
          typename std::enable_if<!IsPointer>::type * = nullptr>
void destroy_if(VectorT & vector)
{
    JSTD_UNUSED(vector);
    /* Do nothing !! */
}

} // namespace detail

template <typename Key, typename Value, bool kValueIsPointer = false>
class ArrayHashmap {
public:
    using size_type = std::size_t;

    using key_type = Key;
    using value_type = Value;
    using element_type = typename std::remove_pointer<Value>::type;
    using ident_type = size_type; 

    using array_type = std::vector<value_type>;
    using hashmap_type = std::unordered_map<key_type, ident_type>;

    using iterator = typename hashmap_type::iterator;
    using const_iterator = typename hashmap_type::const_iterator;

    static constexpr const size_type npos = static_cast<size_type>(-1);
    static constexpr const bool kValueIsPointer = std::is_pointer<value_type>::value; 

protected:
    array_type   array_;
    hashmap_type hashmap_;

    void destroy() {
        detail::destroy_if(array_);
    }

public:
    ArrayHashmap() = default;
    ~ArrayHashmap() {
        destroy();
    }

    size_type size() const { return array_.size(); }
    size_type max_id() const {
        return static<size_type>(array_.size() - 1);
    }

    value_type & get(size_type index) {
        assert(index< array_.size());
        return array_[index];
    }

    const value_type & get(size_type index) const {
        assert(index< array_.size());
        return array_[index];
    }

    void set(size_type index, const value_type & value) {
        assert(index< array_.size());
        array_[index] = value;
    }

    void set(size_type index, value_type && value) {
        assert(index< array_.size());
        array_[index] = std::move(value);
    }

    void clear() {
        array_.clear();
    }

    void clearMap() {
        hashmap_.clear();
    }

    size_type registerName(const key_type & key) {
        auto iter = hashmap_.find(key);
        if (iter != hashmap_.end()) {
            ident_type id = iter->second;
            return static_cast<size_type>(id);
        } else {
            ident_type id = static_cast<ident_type>(array_.size());

            auto result = hashmap_.emplace(key, id);
            if (result.second)
                return id;
            else
                return npos;
        }
    }

    bool isExists(const key_type & key) const {
        auto iter = hashmap_.find(key);
        return (iter != hashmap_.cend());
    }

    iterator find(const key_type & key) {
        return hashmap_.find(key);
    }

    const_iterator find(const key_type & key) const {
        return hashmap_.find(key);
    }

    std::pair<size_type, bool> append(const key_type & name, const key_type & label) {
        auto iter = hashmap_.find(key);
        if (iter != hashmap_.end()) {
            ident_type id = iter->second;
            return { static_cast<size_type>(id), true };
        } else {
            ident_type id = static_cast<ident_type>(array_.size());
            value_type value = detail::construct_if<kValueIsPointer, value_type, element_type>(name, label);
            if (kValueIsPointer)
                array_.push_back(value);
            else
                array_.push_back(std::move(value));

            auto result = hashmap_.emplace(key, id);
            if (result.second)
                return { id, false };
            else
                return { npos, false };
        }
    }

    std::pair<size_type, bool> append(key_type && key, key_type && label) {
        auto iter = hashmap_.find(key);
        if (iter != hashmap_.end()) {
            ident_type id = iter->second;
            return { static_cast<size_type>(id), true };
        } else {
            ident_type id = static_cast<ident_type>(array_.size());
            value_type value = detail::construct_if<kValueIsPointer, value_type, element_type>(
                std::forward<key_type>(name), std::forward<key_type>(label));
            if (kValueIsPointer)
                array_.push_back(value);
            else
                array_.push_back(std::move(value));

            auto result = hashmap_.emplace(key, id);
            if (result.second)
                return { id, false };
            else
                return { npos, false };
        }
    }
};

template <typename Key, typename Value>
class ArrayHashmap<Key, Value, true> {
public:
    using size_type = std::size_t;

    using key_type = Key;
    using value_type = Value;
    using element_type = typename std::remove_pointer<Value>::type;
    using ident_type = size_type; 

    using array_type = std::vector<value_type>;
    using hashmap_type = std::unordered_map<key_type, ident_type>;

    using iterator = typename hashmap_type::iterator;
    using const_iterator = typename hashmap_type::const_iterator;

    static constexpr const size_type npos = static_cast<size_type>(-1);
    static constexpr const bool kValueIsPointer = std::is_pointer<value_type>::value; 

protected:
    array_type   array_;
    hashmap_type hashmap_;

    void destroy() {
        detail::destroy_if(array_);
    }

public:
    ArrayHashmap() = default;
    ~ArrayHashmap() {
        destroy();
    }

    size_type size() const { return array_.size(); }
    size_type max_id() const {
        return static<size_type>(array_.size() - 1);
    }

    value_type get(size_type index) {
        assert(index< array_.size());
        return array_[index];
    }

    const value_type get(size_type index) const {
        assert(index< array_.size());
        return array_[index];
    }

    void set(size_type index, const value_type & value) {
        assert(index< array_.size());
        array_[index] = value;
    }

    void set(size_type index, value_type && value) {
        assert(index< array_.size());
        array_[index] = std::move(value);
    }

    void clear() {
        array_.clear();
    }

    void clearMap() {
        hashmap_.clear();
    }

    size_type registerName(const key_type & key) {
        auto iter = hashmap_.find(key);
        if (iter != hashmap_.end()) {
            ident_type id = iter->second;
            return static_cast<size_type>(id);
        } else {
            ident_type id = static_cast<ident_type>(array_.size());

            auto result = hashmap_.emplace(key, id);
            if (result.second)
                return id;
            else
                return npos;
        }
    }

    bool isExists(const key_type & key) const {
        auto iter = hashmap_.find(key);
        return (iter != hashmap_.cend());
    }

    iterator find(const key_type & key) {
        return hashmap_.find(key);
    }

    const_iterator find(const key_type & key) const {
        return hashmap_.find(key);
    }

    std::pair<size_type, bool> append(const key_type & name, const key_type & label) {
        auto iter = hashmap_.find(key);
        if (iter != hashmap_.end()) {
            ident_type id = iter->second;
            return { static_cast<size_type>(id), true };
        } else {
            ident_type id = static_cast<ident_type>(array_.size());
            value_type value = detail::construct_if<kValueIsPointer, value_type, element_type>(name, label);
            if (kValueIsPointer)
                array_.push_back(value);
            else
                array_.push_back(std::move(value));

            auto result = hashmap_.emplace(key, id);
            if (result.second)
                return { id, false };
            else
                return { npos, false };
        }
    }

    std::pair<size_type, bool> append(key_type && key, key_type && label) {
        auto iter = hashmap_.find(key);
        if (iter != hashmap_.end()) {
            ident_type id = iter->second;
            return { static_cast<size_type>(id), true };
        } else {
            ident_type id = static_cast<ident_type>(array_.size());
            value_type value = detail::construct_if<kValueIsPointer, value_type, element_type>(
                std::forward<key_type>(name), std::forward<key_type>(label));
            if (kValueIsPointer)
                array_.push_back(value);
            else
                array_.push_back(std::move(value));

            auto result = hashmap_.emplace(key, id);
            if (result.second)
                return { id, false };
            else
                return { npos, false };
        }
    }
};

//
// An individual benchmark result.
//
struct BenchmarkResult {
    typedef std::size_t size_type;

    size_type   id;
    std::string hashmap_name;
    std::string blueprint_name;
    size_type   benchmark_id;
    double      average_time;
    double      elasped_time[RUN_COUNT];
    size_type   checksum;

    BenchmarkResult() noexcept : benchmark_id(size_type(-1)), average_time(0.0), checksum(0) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = 0.0;
        }
    }

    BenchmarkResult(const std::string & hashmap_name, const std::string & blueprint_name,
                    size_type benchmark_id, double average_time, size_type checksum)
        : hashmap_name(hashmap_name), blueprint_name(blueprint_name),
          benchmark_id(benchmark_id),
          average_time(average_time), checksum(checksum) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = 0.0;
        }
    }

    BenchmarkResult(const BenchmarkResult & src)
        : hashmap_name(src.hashmap_name), blueprint_name(src.blueprint_name),
          benchmark_id(src.benchmark_id),
          average_time(src.average_time), checksum(src.checksum) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = src.elasped_time[i];
        }
    }

    BenchmarkResult(BenchmarkResult && src)
        : hashmap_name(std::move(src.hashmap_name)),
          blueprint_name(std::move(src.blueprint_name)),
          benchmark_id(src.benchmark_id),
          average_time(src.average_time), checksum(src.checksum) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = src.elasped_time[i];
        }
    }

    void setElapsedTimes(double elasped_time[RUN_COUNT]) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            this->elasped_time[i] = elasped_time[i];
        }
    }

    void swap(BenchmarkResult & other) noexcept {
        if (std::addressof(other) != this) {
            std::swap(this->id,             other.id);
            std::swap(this->hashmap_name,   other.hashmap_name);
            std::swap(this->blueprint_name, other.blueprint_name);
            std::swap(this->benchmark_id,   other.benchmark_id);
            std::swap(this->average_time,   other.average_time);
            std::swap(this->checksum,       other.checksum);
            for (size_type i = 0; i < RUN_COUNT; i++) {
                std::swap(this->elasped_time[i], other.elasped_time[i]);
            }
        }
    }
};

class BenchmarkCategory : public BenchmarkBase,
                          public ArrayHashmap<std::size_t, BenchmarkResult *> {
public:
    typedef std::size_t size_type;

private:
    void destroy() {
        /*
        for (size_type i = 0; i < results_.size(); i++) {
            Result * result = results_[i];
            if (result != nullptr) {
                delete result;
                results_[i] = nullptr;
            }
        }
        results_.clear();
        */
    }

public:
    BenchmarkCategory() : BenchmarkBase(), ArrayHashmap() {}

    BenchmarkCategory(const std::string & name, const std::string & label)
        : BenchmarkBase(name, label), ArrayHashmap() {}

    ~BenchmarkCategory() {
        destroy();
    }

    BenchmarkResult * getResult(size_type index) {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    const BenchmarkResult * getResult(size_type index) const {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    void addResult(size_type benchmark_id,
                   const std::string & name,
                   const std::string & label,
                   const std::string & hashmap_name,
                   const std::string & blueprint_name,
                   double average_time,
                   double elasped_time[RUN_COUNT],
                   size_type checksum) {
        this->id_ = benchmark_id;
        BenchmarkResult result(hashmap_name, blueprint_name, benchmark_id, average_time, checksum);
        result.setElapsedTimes(elasped_time);
    }
};

class BenchmarkHashmap : public BenchmarkBase,
                         public ArrayHashmap<std::string, BenchmarkCategory *> {
public:
    typedef std::size_t size_type;

    BenchmarkHashmap() : BenchmarkBase(), ArrayHashmap() {}

    BenchmarkHashmap(const std::string & name, const std::string & label)
        : BenchmarkBase(name, label), ArrayHashmap() {}

    ~BenchmarkHashmap() {
        destroy();
    }

    BenchmarkCategory * getCategory(size_type index) {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    const BenchmarkCategory * getCategory(size_type index) const {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    BenchmarkCategory * addCategory(const std::string & name, const std::string & label) {
        auto result = append(name, label);
        if (result.first != npos) {
            return get(result.first);
        } else {
            return nullptr;
        }
    }
};

class BenchmarkBluePrint : public BenchmarkBase,
                           public ArrayHashmap<std::string, BenchmarkHashmap *> {
public:
    typedef std::size_t size_type;

    BenchmarkBluePrint() : BenchmarkBase(), ArrayHashmap() {}

    BenchmarkBluePrint(const std::string & name, const std::string & label)
        : BenchmarkBase(name, label), ArrayHashmap() {}

    ~BenchmarkBluePrint() {
        destroy();
    }

    BenchmarkHashmap * getHashmap(size_type index) {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    const BenchmarkHashmap * getHashmap(size_type index) const {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    BenchmarkHashmap * addHashmap(const std::string & name, const std::string & label) {
        auto result = append(name, label);
        if (result.first != npos) {
            return get(result.first);
        } else {
            return nullptr;
        }
    }

    void addResult(size_type benchmark_id,
                   const std::string & hashmap_name,
                   const std::string & blueprint_name,
                   double average_time,
                   double elasped_time[RUN_COUNT],
                   size_type checksum) {
        this->id_ = benchmark_id;
        BenchmarkResult result(hashmap_name, blueprint_name, benchmark_id, average_time, checksum);
        result.setElapsedTimes(elasped_time);
        //categorys_.push_back(std::move(result));
    }
};

class BenchmarkResults : public BenchmarkBase,
                         public ArrayHashmap<std::string, BenchmarkBluePrint *> {
public:
    typedef std::size_t size_type;

private:
    /*
    void destroy() {
        for (size_type i = 0; i < size(); i++) {
            BenchmarkBluePrint * blueprint = get(i);
            if (blueprint != nullptr) {
                delete blueprint;
                set(i, nullptr);
            }
        }
        clear();
    }
    //*/

public:
    BenchmarkResults() : BenchmarkBase(), ArrayHashmap() {}

    BenchmarkResults(const std::string & name, const std::string & label)
        : BenchmarkBase(name, label), ArrayHashmap() {}

    ~BenchmarkResults() {
        destroy();
    }

    BenchmarkBluePrint * getBluePrint(size_type index) {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    const BenchmarkBluePrint * getBluePrint(size_type index) const {
        if (index < size())
            return get(index);
        else
            return nullptr;
    }

    BenchmarkBluePrint * addBluePrint(const std::string & name, const std::string & label) {
        auto result = append(name, label);
        if (result.first != npos) {
            return get(result.first);
        } else {
            return nullptr;
        }
    }

    BenchmarkBluePrint * addBlankLine() {
        return addBluePrint("_blank", "_blank");
    }

    std::string formatMsTime(double fMillisec) {
        char time_buf[256];

        if (fMillisec >= 1000.0 * 60.0 * 30.0) {
            snprintf(time_buf, sizeof(time_buf), "%7.2f Min", fMillisec / (60.0 * 1000.0));
        }
        else if (fMillisec >= 1000.0 * 10.0) {
            snprintf(time_buf, sizeof(time_buf), "%7.2f Sec", fMillisec / 1000.0);
        }
        else if (fMillisec >= 1.0 * 1.0) {
            snprintf(time_buf, sizeof(time_buf), "%7.2f ms ", fMillisec);
        }
        else if (fMillisec >= 0.001 * 10.0) {
            snprintf(time_buf, sizeof(time_buf), "%7.2f us ", fMillisec * 1000.0);
        }
        else {
            snprintf(time_buf, sizeof(time_buf), "%7.2f ns ", fMillisec * 1000000.0);
        }

        return std::string(time_buf);
    }

    /*******************************************************************************************************
       Test                                          std::unordered_map         jstd::Dictionary     Ratio
      ------------------------------------------------------------------------------------------------------
       hash_map<std::string, std::string>          checksum     time         checksum    time

       hash_map<K, V>/find                    | 98765432109   100.00 ms | 98765432109   30.00 ms |   3.33
       hash_map<K, V>/insert                  | 98765432109   100.00 ms | 98765432109   30.00 ms |   3.33
       hash_map<K, V>/emplace                 | 98765432109   100.00 ms | 98765432109   30.00 ms |   3.33
       hash_map<K, V>/erase                   | 98765432109   100.00 ms | 98765432109   30.00 ms |   3.33
      ------------------------------------------------------------------------------------------------------
    *******************************************************************************************************/
    void printResults(const std::string & filename, double totalElapsedTime = 0.0) const {
        for (size_type blueprintId = 0; blueprintId < size(); blueprintId++) {
            const BenchmarkBluePrint * blueprint = getBluePrint(blueprintId);
            if (blueprint != nullptr) {
                printf(" Test                                    %23s   %23s      Ratio\n",
                       this->name_.c_str(), this->label_.c_str());
                printf("--------------------------------------------------------------------------------------------------------\n");
                printf("\n");
                if (blueprint->name().size() <= 40)
                    printf(" %-40s    checksum    time          checksum    time\n", blueprint->name().c_str());
                else
                    printf(" %-52s"          "    time          checksum    time\n", blueprint->name().c_str());
                printf("\n");

                size_type hashmap_count = blueprint->size();
                for (size_type hashmap_id = 0; hashmap_id < hashmap_count; hashmap_id++) {
                    const BenchmarkHashmap * hashmap = blueprint->getHashmap(hashmap_id);
                    double ratio;
                    if (result.elasped_time[0] != 0.0)
                        ratio = result.average_time / result.elasped_time[0];
                    else
                        ratio = 0.0;
                    if (result.name != "_blank") {
                        printf(" %-38s | %11" PRIuPTR " %11s | %11" PRIuPTR " %11s |   %0.2f\n",
                               result.name.c_str(),
                               result.checksum, formatMsTime(result.average_time).c_str(),
                               result.checksum, formatMsTime(result.average_time).c_str(),
                               ratio);
                    }
                    else {
                        printf("\n");
                    }
                }

                if (blueprintId < (size() - 1))
                    printf("\n\n");
            }
        }

        printf("\n");
        printf("--------------------------------------------------------------------------------------------------------\n");
        printf("\n");
        if (filename.size() == 0 || filename.c_str() == nullptr || filename == "")
            printf("Dict filename: %-52s  Total elapsed time: %0.2f ms\n", "header_fields[]", totalElapsedTime);
        else
            printf("Dict filename: %-52s  Total elapsed time: %0.2f ms\n", filename.c_str(), totalElapsedTime);
        printf("\n");
    }
};

} // namespace jtest

#endif // JTEST_BENCHMARK_H
