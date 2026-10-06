# NetDisk 个人网盘系统

一个基于 **C/S 架构**的个人网盘系统，采用 **Qt 客户端 + C++ Linux 服务端**，通过**自定义 TCP 二进制协议**通信。支持文件的上传、下载、秒传、断点续传、分享、回收站、收藏与搜索等完整功能。

## 功能特性

- **注册 / 登录**：手机号 + 密码注册登录，服务端校验身份
- **文件上传 / 下载**：4096 字节分块传输，支持单文件与文件夹下载
- **秒传**：基于 MD5 校验，重复文件直接秒传，避免重复存储与传输
- **断点续传**：上传 / 下载支持续传，记录传输进度，中断后可恢复
- **新建文件夹**：支持目录管理
- **文件分享**：生成分享链接，他人可通过链接访问分享文件
- **回收站**：删除采用软删除，可进入回收站查看并恢复
- **收藏**：收藏常用文件，支持查看与取消收藏
- **文件搜索**：按关键字搜索网盘内文件

## 技术栈与架构

| 层级 | 技术 |
| ---- | ---- |
| 客户端 | Qt 5.12.11 + C++11（Windows，MinGW 7.3.0 32 位），SQLite 缓存任务状态 |
| 服务端 | C++（Linux），epoll 多路复用 + 线程池 + MySQL |
| 通信协议 | 自定义 TCP 二进制协议（`PackType` 协议头 + 柔性数组变长数据） |
| 数据存储 | 服务端 MySQL（库名 `NetDisk`），客户端 SQLite |

服务端采用 **epoll（LT 模式）** 管理大量并发连接，配合**线程池动态扩缩容**处理业务请求，实现高并发下的文件传输。

## 目录结构

```
NetDisk/
├── NetDisk/                    # 服务端源码
│   ├── include/                # 服务端头文件（epoll、线程池、协议、MySQL 封装）
│   └── src/                    # 服务端实现 + makefile
├── netapi/                     # 网络通信层（TCP/UDP、Mediator 中介者）
│   ├── mediator/
│   └── net/
├── sqlapi/                     # 客户端 SQLite 封装
├── md5/                        # MD5 计算
├── face/ images/ tb/           # 界面资源图片
├── ckernel.cpp / ckernel.h     # 客户端核心逻辑
├── maindialog.cpp / logindialog.cpp  # 客户端界面
├── main.cpp                    # 客户端入口
├── common.h                    # 客户端通用文件信息结构
└── NetDisk.pro                 # 客户端 Qt 工程文件
```

## 环境依赖

**服务端（Linux）：**

- g++（支持 C++11）
- MySQL 客户端库（`libmysqlclient`）
- pthread

**客户端（Windows）：**

- Qt 5.12.11（MinGW 7.3.0 32 位）
- SQLite

## 构建与运行

### 服务端（Linux）

```bash
cd NetDisk/src
make
./server 8000        # 端口可省略，默认 8000
```

### 客户端（Windows）

```bash
# 配置 Qt 环境变量后
qmake NetDisk.pro -spec win32-g++ "CONFIG+=release"
mingw32-make
```

运行生成的可执行文件即可连接服务端。

## 数据库配置

服务端使用 MySQL 数据库，库名为 `NetDisk`，核心表包括用户表、文件表、用户文件表、收藏表等，并配套视图与触发器。

数据库连接信息在 `NetDisk/include/packdef.h` 中配置：

```cpp
#define _DEF_DB_NAME    "NetDisk"
#define _DEF_DB_IP      "localhost"
#define _DEF_DB_USER    "root"
#define _DEF_DB_PWD     "666666"
```

> 使用前请先在 MySQL 中创建 `NetDisk` 数据库及相应表结构，并确保数据库连接信息与本地环境一致。

## 自定义协议说明

客户端与服务端通过自定义 TCP 二进制协议通信，协议包以 `PackType`（`10000` 基准 + 偏移）标识类型，并使用**柔性数组**承载变长数据（如文件列表、分享文件数组等），实现紧凑高效的报文传输。

## 项目亮点

- 基于 epoll + 线程池的高并发服务端架构
- MD5 秒传与断点续传，减少重复传输、提升体验
- 自定义二进制协议 + 柔性数组，报文紧凑高效
- 软删除 + 回收站设计，数据可恢复
- 客户端 SQLite 缓存任务状态，保证传输任务的可靠性
