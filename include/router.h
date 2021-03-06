#ifndef __ROUTER_H__
#define __ROUTER_H__

#include <worker_group.h>
#include <map>
#include <boost/thread/shared_mutex.hpp>

namespace reba
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
    boost::shared_mutex host_map_shared_mutex_;
};
} // namespace reba
#endif