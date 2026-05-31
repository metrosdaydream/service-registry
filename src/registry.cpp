#include "registry.h"

#include <algorithm>
#include <ctime>

void Registry::registerInstance(const ServiceInstance& instance) {
    std::lock_guard<std::mutex> lock(mtx_);

    ServiceInstance stored = instance;
    stored.lastHeartbeat = std::time(nullptr);
    if (stored.status.empty()) {
        stored.status = "UP";
    }

    auto& instances = services_[stored.name];
    auto existing = std::find_if(instances.begin(), instances.end(),
                                 [&stored](const ServiceInstance& item) {
                                     return item.ip == stored.ip && item.port == stored.port;
                                 });

    if (existing != instances.end()) {
        *existing = stored;
    } else {
        instances.push_back(stored);
    }
}

std::vector<ServiceInstance> Registry::getAllServices() {
    std::lock_guard<std::mutex> lock(mtx_);

    std::vector<ServiceInstance> result;
    for (const auto& entry : services_) {
        result.insert(result.end(), entry.second.begin(), entry.second.end());
    }
    return result;
}

bool Registry::heartbeat(const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx_);

    auto found = services_.find(name);
    if (found == services_.end()) {
        return false;
    }

    const std::time_t now = std::time(nullptr);
    for (auto& instance : found->second) {
        instance.lastHeartbeat = now;
    }
    return true;
}

