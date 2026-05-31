# 服务注册中心 (Service Registry Center)

基于 C++17 + Vue 3 构建的轻量级服务注册中心 MVP，模拟 Nacos/Eureka 核心极简能力。

> **协作说明**: 本项目采用人机协作模式完成 — 数据结构设计、API 规范、前端交互方案由编码者输出，Codex CLI 按设计方案补齐业务逻辑代码，最终由编码者逐项审查把关质量。详见 [AI_PROMPTS.md](./AI_PROMPTS.md)。

## 环境要求

- **编译器**: g++ 9+ (支持 C++17)
- **构建工具**: CMake 3.10+
- **浏览器**: 现代浏览器 (Chrome/Firefox/Edge)

## 快速启动

### 1. 编译

```bash
cd service-registry
cmake -S . -B build
cmake --build build -j$(nproc)
```

### 2. 启动注册中心

```bash
./build/registry-server
```

终端输出:
```
Service registry server listening on http://localhost:3000
```

### 3. 浏览器访问

打开 http://localhost:3000 查看服务注册中心仪表盘。

### 4. 运行示例客户端

新开一个终端:
```bash
cd service-registry
./build/demo-client
```

正常输出:
```
Registry response [200]: {"code":0,"message":"registered"}
```

## API 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/api/register` | 服务注册 |
| GET | `/api/services` | 查询所有已注册服务 |
| POST | `/api/heartbeat` | 心跳上报（预留） |

### 服务注册请求示例

```json
{
  "name": "user-service",
  "ip": "127.0.0.1",
  "port": 8081,
  "interfaces": [
    { "name": "用户信息查询", "path": "/api/user/info", "method": "GET" },
    { "name": "用户创建", "path": "/api/user/create", "method": "POST" }
  ]
}
```

## 项目结构

```
service-registry/
├── CMakeLists.txt              # 顶层构建文件
├── **config.json**             # ⭐ 项目配置文件（端口/地址/服务信息）
├── src/
│   ├── main.cpp                # HTTP Server（cpp-httplib）
│   ├── registry.h              # 数据结构 + Registry 类声明
│   └── registry.cpp            # Registry 类实现
├── third_party/
│   ├── httplib.h               # Header-only HTTP 库
│   └── json.hpp                # Header-only JSON 库
├── frontend/
│   └── index.html              # Vue 3 单页应用
├── demo-client/
│   ├── CMakeLists.txt
│   └── main.cpp                # 示例微服务客户端（自动注册）
├── README.md                   # 本文件
├── ARCHITECTURE.md             # 架构设计文档
└── AI_PROMPTS.md               # AI 协同研发报告
```

## 配置文件说明

`config.json` 位于项目根目录，无需修改即可直接启动：

```json
{
  "server": {
    "host": "0.0.0.0",       # 监听地址
    "port": 3000,             # 端口
    "frontend": "frontend/index.html"  # 前端页面路径
  },
  "demo_service": {
    "registry_host": "127.0.0.1",  # 注册中心地址
    "registry_port": 3000,         # 注册中心端口
    "name": "demo-service",        # 示例服务名
    "ip": "127.0.0.1",             # 示例服务IP
    "port": 9090,                  # 示例服务端口
    "interfaces": [...]            # 示例服务接口列表
  }
}
```

修改 `port` 即可更改端口（如改为 8080），无需修改代码。
