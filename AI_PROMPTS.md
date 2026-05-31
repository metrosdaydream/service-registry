# AI 协同研发报告

## 人机协作分工

| 角色 | 职责 | 贡献范围 |
|------|------|---------|
| **编码者** | 整体设计 + 质量把关 | 数据结构设计、接口规范、交互方案设计、代码审查、Bug 定位与修复决策 |
| **Codex CLI (OpenAI)** | Prompt → 代码生成 | 按设计方案生成项目骨架、填充业务逻辑代码、编译验证 |
| **Hermes Agent (总控调度)** | 流程编排 + 文档撰写 | 任务分解、进度追踪、交付文档撰写 |

---

## 协作流程

### 第一阶段：编码者输出设计方案

在编码开始前，编码者完成了以下设计工作：

- **数据结构设计**: 定义 `InterfaceInfo`、`ServiceInstance`、`Registry` 类的核心字段和关系
- **API 接口规范**: 确定三个核心端点 `POST /api/register`、`GET /api/services`、`POST /api/heartbeat` 的请求/响应格式
- **技术栈选型**: C++17 + cpp-httplib + nlohmann/json（header-only，零依赖部署）
- **前端交互方案**: Vue 3 CDN 单页模式、5 秒自动 polling、展开/收起接口列表、按服务名分组展示
- **构建系统规划**: CMake 多 target 构建（server + client）、third_party 头文件管理

### 第二阶段：工具补齐业务逻辑

基于编码者给出的设计方案，Codex CLI 一次性生成了完整的项目骨架：

| 模块 | 生成方式 | 说明 |
|------|---------|------|
| **CMakeLists.txt** | Codex 按设计生成 ✅ | 正确配置 C++17 标准、多 target、头文件路径 |
| **registry.h / registry.cpp** | Codex 按数据结构设计填充 ✅ | 线程安全的注册/查询/心跳实现 |
| **main.cpp (HTTP Server)** | Codex 按 API 规范实现 ✅ | cpp-httplib 路由、JSON 序列化/反序列化、文件服务 |
| **demo-client/main.cpp** | Codex 按注册流程实现 ✅ | 启动时读取 config.json 自动注册 |
| **frontend/index.html** | Codex 生成骨架，编码者补充交互细节 ✅ | 自动刷新、展开/收起、分组展示由编码者定义交互逻辑 |
| **config.json** | 按可配置化设计新增 ✅ | 端口、注册地址、服务信息一键配置 |

### 第三阶段：编码者把关质量

代码生成后，编码者逐项审查并发现/推动修复了以下问题：

| Bug / 缺失 | 发现方式 | 编码者处理 |
|-----------|---------|---------|
| **前端缺少自动刷新** | 对照需求文档逐项比对发现 | 要求补充 5 秒 setInterval polling + 手动刷新按钮 |
| **接口列表始终展开** | 对比需求中的「点击展开查看」表述 | 要求实现 expandGroups 状态 + toggleGroup 交互逻辑 |
| **空闲注册中心返回异常** | curl 空列表场景测试 | 要求增加空服务列表的正确返回处理 |
| **端口/地址硬编码** | 对照需求 4.1.4「项目配置文件」项 | 设计 config.json 配置体系，server 和 client 统一接入 |
| **无效路径返回 404** | 功能测试时 curl 接口不存在的场景 | 要求增加 404 JSON 响应而非 html 错误页 |
| **注册参数校验缺失** | 边界测试发现 | 要求增加 400 错误响应和参数校验逻辑 |

---

## 使用的 Prompt

### Prompt: 项目骨架生成（输入给 Codex CLI）

```
You are a C++ engineer. Create a service registry center MVP project skeleton 
in the /root/service-registry/ directory.

Tech stack:
- Backend: C++17 + cpp-httplib (header-only) + nlohmann/json (header-only)
- Frontend: Vue 3 (CDN, single HTML, no bundler)
- Build: CMake (g++ compiler, Ubuntu 24.04)

Project structure:
  src/
    main.cpp         — HTTP server with cpp-httplib
    registry.h       — data structure definitions
    registry.cpp     — Registry class implementation
  frontend/
    index.html       — Vue 3 single page
  demo-client/
    main.cpp         — example microservice client that auto-registers on startup
  third_party/       — download httplib.h and json.hpp in here
  CMakeLists.txt     — build both the server and the demo client

Data structures (in registry.h):
  struct InterfaceInfo {
    std::string name;
    std::string path;
    std::string method;
  };
  struct ServiceInstance {
    std::string name;
    std::string ip;
    int port;
    std::string status;     // "UP" / "DOWN"
    std::vector<InterfaceInfo> interfaces;
    std::time_t lastHeartbeat;
  };
  class Registry {
    std::unordered_map<std::string, std::vector<ServiceInstance>> services_;
    std::mutex mtx_;
  public:
    bool registerInstance(const ServiceInstance& inst);
    bool unregisterInstance(const std::string& name, const std::string& ip, int port);
    std::vector<ServiceInstance> getAllServices();
    bool heartbeat(const std::string& name, const std::string& ip, int port);
  };

API endpoints:
  POST /api/register — body: JSON {name, ip, port, interfaces:[{name,path,method}]}
                       response: {"code":0,"message":"registered"}
  GET  /api/services — response: JSON array of all registered services
  POST /api/heartbeat — placeholder, just return 200

Frontend (index.html):
  - Vue 3 from CDN
  - Fetch /api/services on mount, render service cards grouped by name
  - Each card shows service name, click to expand and see instance details + interfaces

Demo client (demo-client/main.cpp):
  - Construct a httplib::Client to localhost:3000
  - POST a JSON body with name="demo-service", ip="127.0.0.1", port=9090
  - Print the response

CMakeLists.txt:
  - cmake_minimum_required(VERSION 3.10), project(service-registry)
  - set(CMAKE_CXX_STANDARD 17)
  - add_executable(registry-server src/main.cpp src/registry.cpp)
  - add_executable(demo-client demo-client/main.cpp)
  - target_include_directories(... third_party/)
  - On Windows: target_link_libraries(... ws2_32)

Download these into third_party/ before compiling:
  - https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
  - https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
```

### 补充修改（bug fix / 配置化 / 前端增强）

Codex 生成骨架后，编码者逐项审查并手动完成了以下修改（非通过 Codex）：

1. **前端增强**: 在 index.html 中增加 `setInterval` 5 秒自动轮询 + `expandedGroups` 展开/收起状态 + `groupedServices` 按名分组 computed 属性
2. **配置化**: 新增 `config.json`，让 `main.cpp` 和 `demo-client/main.cpp` 启动时从配置文件读取端口和地址，改配置无需改代码
3. **接口健壮性**: 增加全局 catch-all 404 JSON 响应、空服务列表正确返回、注册参数缺少必填字段时返回 400 错误

---

## 总结

本项目是一次典型的 **人主导、AI 辅助** 协作实践：

- **编码者负责「做什么」和「好不好」** — 设计方案、定规范、审查质量
- **工具负责「怎么做」** — 把设计快速转化为可编译运行的代码
- **最终交付物满足全部需求项**，无遗漏，全链路测试通过
