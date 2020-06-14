#ifndef __ROUTER_H__
#define __ROUTER_H__

#include <worker_group.h>
#include <map>
#include <mutex>

namespace lake
{
class Router
{
public:
    Router();
    bool addHost(const std::string &host, engine::NativeBind *worker_group);
    bool removeHost(const std::string &host);
    engine::NativeBind *route_by_host(const std::string &host);
private:
    std::map<const std::string, engine::NativeBind *> host_map_;
    std::mutex host_map_mutex_;
};

extern Router g_router;
} // namespace lake
#endif