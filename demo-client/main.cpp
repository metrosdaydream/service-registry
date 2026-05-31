#include "httplib.h"
#include "json.hpp"

#include <iostream>
#include <fstream>

using json = nlohmann::json;

int main() {
    // 从 config.json 读取配置
    std::string registryHost = "127.0.0.1";
    int registryPort = 3000;
    json payload = {
        {"name", "demo-service"},
        {"ip", "127.0.0.1"},
        {"port", 9090},
        {"interfaces", json::array({
            {{"name", "List Users"}, {"path", "/api/users"}, {"method", "GET"}},
            {{"name", "Create User"}, {"path", "/api/users"}, {"method", "POST"}},
        })}
    };

    std::ifstream cfgFile("config.json");
    if (cfgFile.is_open()) {
        try {
            json cfg = json::parse(cfgFile);
            if (cfg.contains("demo_service")) {
                auto& ds = cfg["demo_service"];
                registryHost = ds.value("registry_host", registryHost);
                registryPort = ds.value("registry_port", registryPort);
                payload["name"] = ds.value("name", payload["name"]);
                payload["ip"] = ds.value("ip", payload["ip"]);
                payload["port"] = ds.value("port", payload["port"]);
                if (ds.contains("interfaces") && ds["interfaces"].is_array()) {
                    payload["interfaces"] = ds["interfaces"];
                }
            }
        } catch (...) {
            std::cerr << "Warning: config.json parse failed, using defaults" << std::endl;
        }
    }

    httplib::Client client(registryHost, registryPort);
    auto response = client.Post("/api/register", payload.dump(), "application/json");

    if (!response) {
        std::cerr << "Failed to connect to registry server at "
                  << registryHost << ":" << registryPort << std::endl;
        return 1;
    }

    std::cout << "Registry response [" << response->status << "]: " << response->body << std::endl;
    return response->status >= 200 && response->status < 300 ? 0 : 1;
}
