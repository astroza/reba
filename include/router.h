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
    bool addHost(std::string &host, engine::NativeBind *worker_group);
    bool removeHost(std::string &host);
private:
    std::map<std::string, engine::NativeBind *> host_map_;
    std::mutex host_map_mutex_;
};

extern Router global_router_;
} // namespace lake
#endif