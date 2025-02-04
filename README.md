# jstd_hashmap

| [中文版](./README.md) | [English Version](./README.en.md) |

## Chinese / 中文版

`jstd` 类库的高性能哈希表：

* `jstd::robin_hash_map`：采用 robin-hood hashing 方法，采用 SIMD 指令 + distance，分段 (Group) 为 32 Bytes，采用 8-bit 的 hash 值， 7-bits 的 distance，剩下的 1-bit 留给了标志位。对整型 key 性能不错，对于大对象的 key, value 性能稍差，在某些场景场景下性能还不错。

* `jstd::group15_flat_map`：采用 SIMD 指令的二次探测法，原理和实现跟 boost::unordered_flat_map 基本一样，分段 (Group) 为 16 Bytes（其中 1 Bytes overflow bits），采用 8 bit 的 hash 值，8 bit 的 overflow bits，除了 insert 新元素比它快一点，其他操作都稍微慢一点点，但性能比 jstd::group16_flat_map 稍微好一点。

* `jstd::group16_flat_map`：采用 SIMD 指令的二次探测法，根据 boost::unordered_flat_map 的原理改制，分段 (Group) 为 16 Bytes，采用 7 bit 的 hash 值，每个 byte 最高位为 overflow bits，即每个分段有 16 bits overflow，性能比 jstd::group15_flat_map、boost::unordered_flat_map 稍差一点。

关键技术：Cache Friendly

## 更新历史

- TODO:

## 编译和使用方法

### 1. 克隆 Git 仓库

```bash
git clone https://gitee.com/shines77/jstd_hashmap.git
# 或者
git clone https://github.com/shines77/jstd_hashmap.git
```

### 2. 配置与编译

切换到本仓库的根目录，然后执行下列命令：

```shell
mkdir build
cd build
cmake ..
make
```

### 3. 运行 benchmark

请先切换到你的 build 目录下再执行下列命令，例如：`./build`。

```bash
# 根据 Jackson Allan 的基准测试改进而来
./bin/jackson_bench


# 根据 Google sprasehash 开源库改进的基准测试

# 默认迭代次数为 10000000（无参数时）
./bin/time_hash_map_new

# 迭代次数 8000000
./bin/time_hash_map_new 8000000

# 只测试 (K = std::string, V = std::string)，节约时间
./bin/time_hash_map_new string


# 低、中、高、超高 - 基数测试，主要测试 insert() 和 find()
./bin/cardinal_bench


# 以下测试已不更新，不推荐

# 跟 Google sprasehash 开源库类似的测试，请尝试新版本 time_hash_map_new

# 默认迭代次数为 10000000（无参数时）
./bin/time_hash_map

# 迭代次数 8000000
./bin/time_hash_map 8000000

# 小数据集测试
./bin/benchmark

# 中数据集测试
./bin/benchmark ./data/Maven.keys.txt
```

### 4. 其他脚本

下列脚本请拷贝到你的 build 目录下再执行，例如：`./build`。

1. `cmake-clean.sh`

清理 `cmake` 的缓存和编译结果（便于重新配置和编译）：

```bash
./cmake-clean.sh
```
