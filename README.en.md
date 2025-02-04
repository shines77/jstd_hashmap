# jstd_hashmap

| [中文版](./README.md) | [English Version](./README.en.md) |

## English / 英文版

High performance hash table: jstd::robin_hash_map,

High performance hash table for `jstd` library:

* `jstd:: robin_hash_map`: Using the robin-hood hashing method, SIMD instruction + distance, with a group size of 32 bytes, and 8-bit hash value, and 7-bit distance value, the remaining 1-bit is reserved for the flag bit. The performance is good for integer keys, but for keys for large objects, The value performance is slightly poor, but it performs quite well in certain scenarios.

* `jstd:: group15_flat_map`: Using SIMD instructions and quadratic probing method, the principle and the implementation are similar to boost:unordered_flat_map, is basically the same. The segmentation (Group) is 16 bytes (including 1 byte overflow bit), using 8-bit hash values and 8-bit overflow bits. Except for the insert new element which is slightly faster than it, all other operations are slightly slower, but the performance is better than jstd:group16_flat_map.

* `jstd:: group16_flat_map`: Using SIMD instructions and quadratic probing method, based on boost::unordered_flat_map, the principle of boost::unordered_flat_map is modified. With a group size of 16 bytes and a 7-bit hash value, the highest bit of each byte is overflow bits, which means each segment has 16 bits of overflow. The performance is slightly worse than jstd::group15_flat_map and boost::unordered_flat_map.

Key technology: Cache Friendly

## ChangeLog

- TODO:

## Compile and usage

### 1. Clone git repository

```bash
git clone https://github.com/shines77/jstd_hashmap.git
or
git clone https://gitee.com/shines77/jstd_hashmap.git
```

### 2. Configure and Compile

Change to root dir this repository, and then execute the following command:

```shell
mkdir build
cd build
cmake ..
make
```

### 3. Run benchmarks

Please switch to your build directory first and then execute the following command, for example: `/ build` .

```bash
# Improved based on Jackson Allan's banchmark testing
./bin/jackson_bench


# Improved benchmark testing based on Google sprasehash benchmark code

# Default iteraters is 10000000 (no argument)
./bin/time_hash_map_new

# Set iteraters = 8000000
./bin/time_hash_map_new 8000000

# Only test (K = std::string, V = std::string), to save time
./bin/time_hash_map_new string


# Small, Middle, Big, Huge - Cardinal benchmark, mainly for testing insert() and find()
./bin/cardinal_bench


# The following benchmar tests are no longer updated and are not recommended.

# like Google sprasehash benchmark code

# Default iteraters is 10000000 (no argument)
./bin/time_hash_map

# Set iteraters = 8000000
./bin/time_hash_map 8000000

# Small test case
./bin/benchmark

# Middle test case
./bin/benchmark ./data/Maven.keys.txt
```

### 4. Other script

Please copy the following script to your build directory before executing it, for example: `/ build`。

1. `cmake-clean.sh`

Clean up the cache and compilation results of cmake (easy to reconfigure and compile).

You can use following command:

```bash
./cmake-clean.sh
```
