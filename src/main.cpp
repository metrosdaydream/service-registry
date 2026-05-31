#include "registry.h"

#include "httplib.h"
#include "json.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace {

Registry registry;

// 配置
struct Config {
    std::string host = "0.0.0.0";
    int port = 3000;
    std::string frontend = "frontend/index.html";
};

Config loadConfig() {
    Config cfg;
    std::ifstream file("config.json");
    if (file.is_open()) {
        try {
            json j = json::parse(file);
            if (j.contains("server")) {
                cfg.host = j["server"].value("host", cfg.host);
                cfg.port = j["server"].value("port", cfg.port);
                cfg.frontend = j["server"].value("frontend", cfg.frontend);
            }
        } catch (...) {
            std::cerr << "Warning: config.json parse failed, using defaults" << std::endl;
        }
    }
    // 环境变量优先
    const char* env = std::getenv("SERVICE_REGISTRY_FRONTEND");
    if (env != nullptr && std::string(env).size() > 0) {
        cfg.frontend = env;
    }
    return cfg;
}

InterfaceInfo parseInterface(const json& item) {
    InterfaceInfo info;
    info.name = item.value("name", "");
    info.path = item.value("path", "");
    info.method = item.value("method", "GET");
    return info;
}

ServiceInstance parseServiceInstance(const json& body) {
    ServiceInstance instance;
    instance.name = body.value("name", "");
    instance.ip = body.value("ip", "");
    instance.port = body.value("port", 0);
    instance.status = body.value("status", "UP");
    instance.lastHeartbeat = std::time(nullptr);

    if (body.contains("interfaces") && body["interfaces"].is_array()) {
        for (const auto& item : body["interfaces"]) {
            instance.interfaces.push_back(parseInterface(item));
        }
    }

    return instance;
}

json interfaceToJson(const InterfaceInfo& info) {
    return json{
        {"name", info.name},
        {"path", info.path},
        {"method", info.method},
    };
}

json serviceToJson(const ServiceInstance& instance) {
    json interfaces = json::array();
    for (const auto& info : instance.interfaces) {
        interfaces.push_back(interfaceToJson(info));
    }

    return json{
        {"name", instance.name},
        {"ip", instance.ip},
        {"port", instance.port},
        {"status", instance.status},
        {"interfaces", interfaces},
        {"lastHeartbeat", instance.lastHeartbeat},
    };
}

json allServicesToJson(const std::vector<ServiceInstance>& services) {
    json result = json::array();
    for (const auto& service : services) {
        result.push_back(serviceToJson(service));
    }
    return result;
}

void setJsonResponse(httplib::Response& res, const json& body, int status = 200) {
    res.status = status;
    res.set_content(body.dump(), "application/json; charset=utf-8");
}

std::string frontendPath() {
    Config cfg = loadConfig();
    return cfg.frontend;
}

bool readFile(const std::string& path, std::string& content) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}

}  // namespace

int main() {
    httplib::Server server;

    server.Post("/api/register", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const json body = json::parse(req.body);
            ServiceInstance instance = parseServiceInstance(body);

            if (instance.name.empty() || instance.ip.empty() || instance.port <= 0) {
                setJsonResponse(res, {{"code", 1}, {"message", "name, ip and port are required"}}, 400);
                return;
            }

            registry.registerInstance(instance);
            setJsonResponse(res, {{"code", 0}, {"message", "registered"}});
        } catch (const std::exception& ex) {
            setJsonResponse(res, {{"code", 1}, {"message", ex.what()}}, 400);
        }
    });

    server.Get("/api/services", [](const httplib::Request&, httplib::Response& res) {
        setJsonResponse(res, {{"code", 0}, {"data", allServicesToJson(registry.getAllServices())}});
    });

    server.Post("/api/heartbeat", [](const httplib::Request&, httplib::Response& res) {
        // TODO: Parse heartbeat payload and update a specific service instance.
        setJsonResponse(res, {{"code", 0}, {"message", "ok"}});
    });

    server.Get(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        // API 路由不存在的返回 404
        if (req.path.find("/api/") == 0) {
            setJsonResponse(res, {{"code", 1}, {"message", "not found"}}, 404);
            return;
        }
        std::string html;
        if (readFile(frontendPath(), html)) {
            res.set_content(std::move(html), "text/html; charset=utf-8");
        } else {
            res.status = 404;
            res.set_content("frontend/index.html not found", "text/plain; charset=utf-8");
        }
    });

    Config cfg = loadConfig();

    std::cout << "====================================" << std::endl;
    std::cout << "  服务注册中心" << std::endl;
    std::cout << "  地址: http://" << cfg.host << ":" << cfg.port << std::endl;
    std::cout << "  API 注册: POST /api/register" << std::endl;
    std::cout << "  API 查询: GET  /api/services" << std::endl;
    std::cout << "  API 心跳: POST /api/heartbeat" << std::endl;
    std::cout << "  前端文件: " << cfg.frontend << std::endl;
    std::cout << "====================================" << std::endl;

    server.listen(cfg.host, cfg.port);
    return 0;
}
