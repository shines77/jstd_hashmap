# jstd_hashmap

| [中文版](./README.md) | [English Version](./README.en.md) |

## Chinese / 中文版

TODO: //

## 编译和使用方法

### 1. 克隆 Git 仓库

```bash
git clone https://gitee.com/shines77/jstd_hashmap.git
# 或者
git clone https://github.com/shines77/jstd_hashmap.git
```

### 2. 配置与编译

切换到本仓库的根目录，执行：

```shell
cmake .
make
```

### 3. 其他脚本

清理 `cmake` 的缓存和编译结果（便于重新配置和编译）：

```bash
./cmake-clean.sh
```

### 4. 运行 benchmark

```bash
# 跟 Google sprasehash 开源库类似的测试

# 默认迭代次数为 10000000（无参数时）
./bin/time_hash_map

# 迭代次数 8000000
./bin/time_hash_map 8000000


# 低、中、高、超高 - 基数测试
./bin/cardinal_bench


# 小数据集测试
./bin/benchmark

# 中数据集测试
./bin/benchmark ./data/Maven.keys.txt
```
