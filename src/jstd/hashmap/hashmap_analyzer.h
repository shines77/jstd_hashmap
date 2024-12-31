
#ifndef JSTD_HASH_HASHMAP_ANALYZER_H
#define JSTD_HASH_HASHMAP_ANALYZER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"
#include "jstd/basic/inttypes.h"

#include <stdio.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <string>
#include <vector>

#include "jstd/traits/type_traits.h"
#include "jstd/string/string_utils.h"

namespace jstd {

template <typename Container>
class JSTD_DLL HashMapAnalyzer {
public:
    typedef Container                           container_type;
    typedef typename Container::size_type       size_type;
    typedef typename Container::key_type        key_type;
    typedef typename Container::mapped_type     mapped_type;

    typedef typename Container::iterator        iterator;
    typedef typename Container::const_iterator  const_iterator;
    typedef typename Container::hasher          hasher;

    struct BaseInfo {
        size_type entry_size;
        size_type entry_capacity;
        size_type bucket_mask;
        size_type bucket_capacity;

        BaseInfo() : entry_size(0), entry_capacity(0),
                     bucket_mask(0), bucket_capacity(0) {}
        ~BaseInfo() {}

        void reset() {
            entry_size = 0;
            entry_capacity = 0;
            bucket_mask = 0;
            bucket_capacity = 0;
        }
    };

    struct Result {
        bool     isInited;
        BaseInfo base;

        size_type max_bucket_count;
        double std_deviation;
        double std_errors;
        double coverage_rate;
        double usage_rate, usage_rate_total;
        double unused_rate, unused_rate_total;
        double conflict_rate, conflict_rate_total;
        double perfect_rate, perfect_rate_total;

        Result() : isInited(false), max_bucket_count(0),
                   std_deviation(0.0), std_errors(0.0),
                   coverage_rate(0.0),
                   usage_rate(0.0), usage_rate_total(0),
                   unused_rate(0.0), unused_rate_total(0.0),
                   conflict_rate(0.0), conflict_rate_total(0.0),
                   perfect_rate(0.0), perfect_rate_total(0.0) {
        }
        ~Result() {}

        void reset() {
            isInited = false;

            base.reset();

            max_bucket_count = 0;

            std_deviation = 0.0;
            std_errors = 0.0;
            coverage_rate = 0.0;
            usage_rate = 0.0;
            usage_rate_total = 0.0;
            unused_rate = 0.0;
            unused_rate_total = 0.0;
            conflict_rate = 0.0;
            conflict_rate_total = 0.0;
            perfect_rate = 0.0;
            perfect_rate_total = 0.0;
        }
    };

private:
    const container_type &  container_;
    std::string             name_;

    std::vector<size_type>  bucket_counts_;
    Result                  result_;

public:
    HashMapAnalyzer(const container_type & container)
        : container_(container) {}
    ~HashMapAnalyzer() {}

    size_type entry_size() const {
        return this->container_.size();
    }

    size_type entry_count() const {
        bool supported = has_entry_count<container_type>::value;
        if (supported)
            return call_entry_count<container_type>::entry_count(this->container_);
        else
            return this->container_.bucket_count();
    }

    size_type bucket_mask() const {
        return (this->container_.bucket_count() - 1);
    }

    size_type bucket_count() const {
        return this->container_.bucket_count();
    }

    size_type bucket_size(size_type n) {
        return this->container_.bucket_size(n);
    }

    std::string & name() {
        if (this->name_.c_str() == nullptr || this->name_.size() == 0) {
            this->name_ = "Unknown HashMap<K, V>";
        }
        return this->name_;
    }

    const std::string &  name() const {
        if (this->name_.c_str() == nullptr || this->name_.size() == 0) {
            this->name_ = "Unknown HashMap<K, V>";
        }
        return this->name_;
    }

    void set_name(const std::string & name) {
        this->name_ = name;
    }

private:
    size_type read_bucket_sizes() {
        size_type bucket_capacity = this->bucket_count();

        this->bucket_counts_.clear();
        this->bucket_counts_.resize(bucket_capacity);

        size_type total_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_size(i);
            this->bucket_counts_[i] = count;
            total_count += count;
        }

        result_.base.entry_size = total_count;
        assert(total_count == this->entry_size());
        return total_count;
    }

    size_type get_max_bucket_count() const {
        size_type bucket_capacity = this->bucket_counts_.size();

        size_type max_bucket_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_counts_[i];
            if (count > max_bucket_count)
                max_bucket_count = count;
        }

        return max_bucket_count;
    }

    double calc_std_deviation() const {
        size_type bucket_capacity = this->bucket_counts_.size();
        if (bucket_capacity == 0) return 0.0;

        size_type total_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_counts_[i];
            total_count += count;
        }

        double average_count = total_count / (double)bucket_capacity;
        double std_errors = 0.0;

        for (size_type i = 0; i < bucket_capacity; i++) {
            double error = (double)this->bucket_counts_[i] - average_count;
            std_errors += error * error;
        }

        std_errors /= (double)bucket_capacity;
        return std::sqrt(std_errors);
    }

    double calc_std_errors() const {
        size_type bucket_capacity = this->bucket_counts_.size();
        if (bucket_capacity == 0) return 0.0;

        size_type total_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_counts_[i];
            total_count += count;
        }

        size_type entry_capacity = entry_count();
        double average_count = total_count / (double)entry_capacity;
        double std_errors = 0.0;

        for (size_type i = 0; i < bucket_capacity; i++) {
            if (this->bucket_counts_[i] > 0) {
                double error = (double)this->bucket_counts_[i] - average_count;
                std_errors += error * error;
            }
        }

        std_errors /= (double)entry_capacity;
        return std::sqrt(std_errors);
    }

    double calc_coverage_rate() const {
        if (this->entry_count() != 0)
            return (((double)this->entry_size() / this->entry_count()) * 100.0);
        else
            return 0.0;
    }

    size_type calc_usage_count() const {
        size_type bucket_capacity = this->bucket_counts_.size();

        size_type usage_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_counts_[i];
            if (count > 0)
                usage_count++;
        }

        return usage_count;
    }

    double calc_usage_rate(size_type usage_count) const {
        double usage_rate;
        if (entry_size() != 0)
            usage_rate = usage_count / (double)entry_size();
        else
            usage_rate = 0.0;
        return (usage_rate * 100.0);
    }

    double calc_usage_rate_total(size_type usage_count) const {
        size_type bucket_capacity = this->bucket_counts_.size();
        double usage_rate_total;
        if (bucket_capacity != 0)
            usage_rate_total = usage_count / (double)bucket_capacity;
        else
            usage_rate_total = 0.0;
        return (usage_rate_total * 100.0);
    }

    double calc_unused_rate(size_type usage_count) const {
        size_type entry_size = this->entry_size();
        double unused_rate;
        if (entry_size != 0)
            unused_rate = (entry_size - usage_count) / (double)entry_size;
        else
            unused_rate = 0.0;
        return (unused_rate * 100.0);
    }

    double calc_unused_rate_total(size_type usage_count) const {
        return (100.0 - calc_usage_rate_total(usage_count));
    }

    size_type calc_conflict_count() const {
        size_type bucket_capacity = this->bucket_counts_.size();
        if (bucket_capacity == 0) return 0;

        size_type conflict_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_counts_[i];
            if (count > 1)
                conflict_count++;
        }

        return conflict_count;
    }

    double calc_conflict_rate(size_type conflict_count) const {
        size_type entry_size = this->entry_size();
        double conflict_rate;
        if (entry_size != 0)
            conflict_rate = conflict_count / (double)entry_size;
        else
            conflict_rate = 0.0;
        return (conflict_rate * 100.0);
    }

    double calc_conflict_rate_total(size_type conflict_count) const {
        size_type bucket_capacity = this->bucket_counts_.size();
        double conflict_rate;
        if (bucket_capacity != 0)
            conflict_rate = conflict_count / (double)bucket_capacity;
        else
            conflict_rate = 0.0;
        return (conflict_rate * 100.0);
    }

    size_type calc_perfect_count() const {
        size_type bucket_capacity = this->bucket_counts_.size();
        if (bucket_capacity == 0) return 0;

        size_type perfect_count = 0;
        for (size_type i = 0; i < bucket_capacity; i++) {
            size_type count = this->bucket_counts_[i];
            if (count == 1)
                perfect_count++;
        }

        return perfect_count;
    }

    double calc_perfect_rate(size_type perfect_count) const {
        size_type entry_size = this->entry_size();
        double perfect_rate;
        if (entry_size != 0)
            perfect_rate = perfect_count / (double)entry_size;
        else
            perfect_rate = 0.0;
        return (perfect_rate * 100.0);
    }

    double calc_perfect_rate_total(size_type perfect_count) const {
        size_type bucket_capacity = this->bucket_counts_.size();
        double perfect_rate;
        if (bucket_capacity != 0)
            perfect_rate = perfect_count / (double)bucket_capacity;
        else
            perfect_rate = 0.0;
        return (perfect_rate * 100.0);
    }

public:
    bool start_analyse() {
        result_.reset();

        size_type total_count = this->read_bucket_sizes();
        if (total_count > 0) {
            result_.base.entry_size      = this->entry_size();
            result_.base.entry_capacity  = this->entry_count();
            result_.base.bucket_mask     = this->bucket_mask();
            result_.base.bucket_capacity = this->bucket_count();

            size_type usage_count       = calc_usage_count();
            size_type conflict_count    = calc_conflict_count();
            size_type perfect_count     = calc_perfect_count();

            result_.max_bucket_count    = get_max_bucket_count();

            result_.std_deviation       = calc_std_deviation();
            result_.std_errors          = calc_std_errors();
            result_.coverage_rate       = calc_coverage_rate();
            result_.usage_rate          = calc_usage_rate(usage_count);
            result_.usage_rate_total    = calc_usage_rate_total(usage_count);
            result_.unused_rate         = calc_unused_rate(usage_count);
            result_.unused_rate_total   = calc_unused_rate_total(usage_count);
            result_.conflict_rate       = calc_conflict_rate(conflict_count);
            result_.conflict_rate_total = calc_conflict_rate_total(conflict_count);
            result_.perfect_rate        = calc_perfect_rate(perfect_count);
            result_.perfect_rate_total  = calc_perfect_rate_total(perfect_count);

            result_.isInited = true;
            return true;
        }

        return false;
    }

    void display_status() {
        printf("--------------------------------------------------------------\n");
        printf("  %s\n", this->name().c_str());
        printf("--------------------------------------------------------------\n");
        printf("\n");

        printf("  entry_size       = %" PRIuPTR "\n", this->result_.base.entry_size);
        printf("  entry_capacity   = %" PRIuPTR "\n", this->result_.base.entry_capacity);
        printf("  bucket_mask      = %" PRIuPTR "\n", this->result_.base.bucket_mask);
        printf("  bucket_capacity  = %" PRIuPTR "\n", this->result_.base.bucket_capacity);
        printf("\n");
        printf("  std_deviation    = %0.6f\n", this->result_.std_deviation);
        printf("  std_errors       = %0.6f\n", this->result_.std_errors);
        printf("  max_bucket_count = %" PRIuPTR "\n", this->result_.max_bucket_count);
        printf("  coverage_rate    = %6.2f %%  - size() / bucket_count()\n",
                                     this->result_.coverage_rate);
        printf("\n");
        printf("  usage_rate       = %6.2f %%  /  %6.2f %% (Total)\n",
                                     this->result_.usage_rate, this->result_.usage_rate_total);
        printf("  unused_rate      = %6.2f %%  /  %6.2f %% (Total)\n",
                                     this->result_.unused_rate, this->result_.unused_rate_total);
        printf("  conflict_rate    = %6.2f %%  /  %6.2f %% (Total)\n",
                                     this->result_.conflict_rate, this->result_.conflict_rate_total);
        printf("  perfect_rate     = %6.2f %%  /  %6.2f %% (Total)\n",
                                     this->result_.perfect_rate, this->result_.perfect_rate_total);
        printf("\n");
    }

    void dump_entries(uint32_t max_entries = 0) {
        printf("\n");
        printf("   #       hash     index     key                            value\n");
        printf("------------------------------------------------------------------------\n");

        uint32_t index = 0;
        hasher _hasher;

        str_utils::string_format<key_type> formatter;
        for (const_iterator iter = container_.cbegin(); iter != container_.cend(); ++iter) {
            std::uint32_t hash_code = static_cast<std::uint32_t>(_hasher(iter->first));
            printf(" [%3d]: 0x%08X  %-8u  %-30s %s\n", index + 1,
                    hash_code,
                    uint32_t(hash_code & this->bucket_mask()),
                    formatter.to_string(iter->first).c_str(),
                    formatter.to_string(iter->second).c_str());
            index++;
            if (max_entries != 0 && index >= max_entries) {
                break;
            }
        }

        printf("\n\n");
    }
};

} // namespace jstd

#endif // JSTD_HASH_HASHMAP_ANALYZER_H
