# jstd_hashmap

| [中文版](./README.md) | [English Version](./README.en.md) |

## English / 英文版

High performance hash table: jstd::robin_hash_map, key technology: Cache Friendly

## Compile and usage

### 1. Clone git repository

```bash
git clone https://github.com/shines77/jstd_hashmap.git
or
git clone https://gitee.com/shines77/jstd_hashmap.git
```

### 2. Configure and Compile

Change to root dir this repository:

```shell
cmake .
make
```

### 3. Other script

Clean up the cache and compilation results of cmake (easy to reconfigure and compile).

You can use following command:

```bash
./cmake-clean.sh
```

### 4. Run benchmark

```bash
# like Google sprasehash benchmark code

# Default iteraters is 10000000 (no argument)
./bin/time_hash_map

# Set iteraters = 8000000
./bin/time_hash_map 8000000


# Small, Middle, Big, Huge - Cardinal benchmark
./bin/cardinal_bench


# Small test case
./bin/benchmark

# Middle test case
./bin/benchmark ./data/Maven.keys.txt
```
