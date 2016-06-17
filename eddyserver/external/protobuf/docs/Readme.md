# 说明
protobuf-2.6.1.tar.gz 为protobuf源代码，最新版本的下载地址为[https://developers.google.com/protocol-buffers/docs/downloads](https://developers.google.com/protocol-buffers/docs/downloads)。

libprotobuf.py 用于提取libprotobuf库中需要的C++源文件，方便cmake添加。使用方式是将protobuf-2.6.1.tar.gz解压，将这个脚本放到protobuf-2.6.1目录下执行，将生成libprotobuf.txt文件，这个文件里记录了libprotobuf库所需的c++源文件。

# 编译libprotobuf
首先将解压protobuf-2.6.1.tar.gz，将vsprojects/config.h和src目录下的google文件夹拷贝出来。如需libprotobuf编译通过必须修改config.h，以下：
1. 修改```#define HASH_MAP_H <hash_map>```宏定义为```#define HASH_MAP_H <unordered_map>```
2. 修改```#define HASH_NAMESPACE stdext```宏定义为```#define HASH_NAMESPACE std```
3. 修改```#define HASH_SET_H <hash_set>```宏定义为```#define HASH_SET_H <hash_set>```
4. 添加宏定义```#define HAVE_PTHREAD 1```
5. 添加宏定义```#define HASH_MAP_CLASS unordered_map```
6. 添加宏定义```#define HASH_SET_CLASS unordered_set```
7. 添加宏定义，防止windows上编译失败
    ```
    #if defined(_MSC_VER)
    #define _STLPORT_VERSION 1
    #endif
    ```