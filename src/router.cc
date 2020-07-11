#include <router.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace reba
{
Router g_router;

Router::Router(){};

bool Router::addHost(const std::string &host, engine::NativeBind *worker_group)
{
    bool new_host = true;
    std::lock_guard<std::mutex> lock(host_map_mutex_);
    auto old_worker_group = host_map_.find(host);
    if(old_worker_group != host_map_.end()) {
        old_worker_group->second->unref();
        new_host = false;
    }
    host_map_[std::move(host)] = worker_group;
    worker_group->ref();
    return new_host;
}

bool Router::removeHost(const std::string &host)
{
    bool host_found = false;
    std::lock_guard<std::mutex> lock(host_map_mutex_);
    auto worker_group = host_map_.find(host);
    if(worker_group != host_map_.end()) {
        worker_group->second->unref();
        host_map_.erase(worker_group);
        host_found = true;
    }
    return host_found;
}

engine::NativeBind *Router::route_by_host(const std::string &host) 
{
    auto worker_group = host_map_.find(host);
    if(worker_group != host_map_.end()) {
        return worker_group->second;
    }
    return nullptr;
}

//engine::NativeBind *Router::route()
//{

//}
} // namespace reba