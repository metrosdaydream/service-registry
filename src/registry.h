#ifndef SERVICE_REGISTRY_REGISTRY_H
#define SERVICE_REGISTRY_REGISTRY_H

#include <ctime>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct InterfaceInfo {
    std::string name;
    std::string path;
    std::string method;
};

struct ServiceInstance {
    std::string name;
    std::string ip;
    int port;
    std::string status;
    std::vector<InterfaceInfo> interfaces;
    std::time_t lastHeartbeat;
};

class Registry {
public:
    void registerInstance(const ServiceInstance& instance);
    std::vector<ServiceInstance> getAllServices();
    bool heartbeat(const std::string& name);

private:
    std::unordered_map<std::string, std::vector<ServiceInstance>> services_;
    std::mutex mtx_;
};

#endif

