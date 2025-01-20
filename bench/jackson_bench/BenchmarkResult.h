
#ifndef JTEST_BENCHMARK_H
#define JTEST_BENCHMARK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"
#include "jstd/basic/inttypes.h"

#include <stdio.h>

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <type_traits>

namespace jtest {

class TableBase {
private:
    size_type   id_;
    std::string name_;
    std::string label_;

public:
    TableBase() : id_(size_type(-1)) {}
    TableBase(const std::string & name, const std::string & label = "")
        : id_(size_type(-1)), name_(name), label_(label) {}

    TableBase(const TableBase & src)
        : id_(src.id()),
          name_(src.name()), label_(src.label()) {}

    TableBase(TableBase && src)
        : id_(src.id()),
          name_(std::move(src.name())), label_(std::move(src.label())) {}

    ~TableBase() {}

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

    void swap(TableBase & other) noexcept {
        if (std::addressof(other) != this) {
            std::swap(this->id_,    other.id_);
            std::swap(this->name_,  other.name_);
            std::swap(this->label_, other.label_);
        }
    }
};

//
// An individual benchmark result.
//
struct Result {
    typedef std::size_t size_type;

    size_type   id;
    std::string hashmap_name;
    std::string blueprint_name;
    size_type   benchmark_id;
    double      average_time;
    double      elasped_time[RUN_COUNT];
    size_type   checksum;

    Result() noexcept : benchmark_id(size_type(-1)), average_time(0.0), checksum(0) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = 0.0;
        }
    }

    Result(const std::string & hashmap_name, const std::string & blueprint_name,
           size_type benchmark_id, double average_time, size_type checksum)
        : hashmap_name(hashmap_name), blueprint_name(blueprint_name),
          benchmark_id(benchmark_id),
          average_time(average_time), checksum(checksum) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = 0.0;
        }
    }

    Result(const Result & src)
        : hashmap_name(src.hashmap_name), blueprint_name(src.blueprint_name),
          benchmark_id(src.benchmark_id),
          average_time(src.average_time), checksum(src.checksum) {
        for (size_type i = 0; i < RUN_COUNT; i++) {
            elasped_time[i] = src.elasped_time[i];
        }
    }

    Result(Result && src)
        : hashmap_name(std:move(src.hashmap_name)),
          blueprint_name(std:move(src.blueprint_name)),
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

    void swap(Result & other) noexcept {
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

class BenchmarkCategory : public TableBase {
public:
    typedef std::size_t size_type;

private:
    std::vector<Result> results_;

    void destroy() {
        for (size_type i = 0; i < results_.size(); i++) {
            Result * result = results_[i];
            if (result != nullptr) {
                delete result;
                results_[i] = nullptr;
            }
        }
        results_.clear();
    }

public:
    BenchmarkCategory() : TableBase() {}

    BenchmarkCategory(const std::string & name, const std::string & label)
        : TableBase(name, label) {}

    ~BenchmarkCategory() {
        destroy();
    }

    size_type size() const { return results_.size(); }

    Result & getResult(size_type index) {
        return results_[index];
    }

    const Result & getResult(size_type index) const {
        return results_[index];
    }

    void addResult(size_type benchmark_id,
                   const std::string & hashmap_name,
                   const std::string & blueprint_name,
                   double average_time,
                   double elasped_time[RUN_COUNT],
                   size_type checksum) {
        this->id_ = benchmark_id;
        Result result(hashmap_name, blueprint_name, benchmark_id, average_time, checksum);
        result.setElapsedTimes(elasped_time);
        results_.push_back(std::move(result));
    }
};

class BenchmarkBluePrint : public TableBase {
public:
    typedef std::size_t size_type;

private:
    std::vector<BenchmarkCategory> categorys_;

    void destroy() {
        for (size_type i = 0; i < categorys_.size(); i++) {
            BenchmarkCategory * category = categorys_[i];
            if (category != nullptr) {
                delete category;
                categorys_[i] = nullptr;
            }
        }
        categorys_.clear();
    }

public:
    BenchmarkBluePrint() : TableBase() {}

    BenchmarkBluePrint(const std::string & name, const std::string & label)
        : TableBase(name, label) {}

    ~BenchmarkBluePrint() {
        destroy();
    }

    size_type size() const {
        return categorys_.size();
    }

    Result & getCategory(size_type index) {
        return categorys_[index];
    }

    const Result & getCategory(size_type index) const {
        return categorys_[index];
    }

    void addResult(size_type benchmark_id,
                   const std::string & hashmap_name,
                   const std::string & blueprint_name,
                   double average_time,
                   double elasped_time[RUN_COUNT],
                   size_type checksum) {
        this->id_ = benchmark_id;
        Result result(hashmap_name, blueprint_name, benchmark_id, average_time, checksum);
        result.setElapsedTimes(elasped_time);
        results_.push_back(std::move(result));
    }
};

class BenchmarkResults : public TableBase {
public:
    typedef std::size_t size_type;

private:
    std::vector<BenchmarkBluePrint *> blueprints_;

    void destroy() {
        for (size_type i = 0; i < blueprints_.size(); i++) {
            BenchmarkBluePrint * blueprint = blueprints_[i];
            if (blueprint != nullptr) {
                delete blueprint;
                blueprints_[i] = nullptr;
            }
        }
        blueprints_.clear();
    }

public:
    BenchmarkResults() : TableBase() {}

    ~BenchmarkResults() {
        destroy();
    }

    size_type size() const {
        return blueprints_.size();
    }

    BenchmarkBluePrint * getBluePrint(size_type index) {
        if (index < blueprints_.size())
            return blueprints_[index];
        else
            return nullptr;
    }

    size_type addBluePrint(const std::string & name, const std::string & label) {
        BenchmarkBluePrint * blueprint = new BenchmarkBluePrint(name, label);
        blueprints_.push_back(blueprint);
        return (blueprints_.size() - 1);
    }

    bool addResult(size_type catId, const std::string & name,
                   double elaspedTime1, size_type checksum1,
                   double elaspedTime2, size_type checksum2) {
        BenchmarkBluePrint * blueprint = getBluePrint(catId);
        if (blueprint != nullptr) {
            blueprint->addResult(catId, name, elaspedTime1, checksum1, elaspedTime2, checksum2);
            return true;
        }
        return false;
    }

    bool addBlankLine(size_type catId) {
        BenchmarkBluePrint * blueprint = getBluePrint(catId);
        if (blueprint != nullptr) {
            blueprint->addResult(catId, "_blank", 0.0, 0, 0.0, 0);
            return true;
        }
        return false;
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
    void printResults(const std::string & filename, double totalElapsedTime = 0.0) {
        for (size_type blueprintId = 0; blueprintId < size(); blueprintId++) {
            BenchmarkBluePrint * blueprint = getBluePrint(blueprintId);
            if (blueprint != nullptr) {
                printf(" Test                                    %23s   %23s      Ratio\n",
                       this->name_.c_str(), this->label_.c_str());
                printf("--------------------------------------------------------------------------------------------------------\n");
                printf("\n");
                if (blueprint->name().size() <= 40)
                    printf(" %-40s    checksum    time          checksum    time\n", category->name().c_str());
                else
                    printf(" %-52s"          "    time          checksum    time\n", category->name().c_str());
                printf("\n");

                size_type result_count = blueprint->size();
                for (size_type i = 0; i < result_count; i++) {
                    const BenchmarkCategory & category = blueprint->getCategory(i);
                    double ratio;
                    if (result.elaspedTime2 != 0.0)
                        ratio = result.elaspedTime1 / result.elaspedTime2;
                    else
                        ratio = 0.0;
                    if (result.name != "_blank") {
                        printf(" %-38s | %11" PRIuPTR " %11s | %11" PRIuPTR " %11s |   %0.2f\n",
                               result.name.c_str(),
                               result.checksum1, formatMsTime(result.elaspedTime1).c_str(),
                               result.checksum2, formatMsTime(result.elaspedTime2).c_str(),
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
