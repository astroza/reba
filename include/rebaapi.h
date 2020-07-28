#ifndef __LAKEAPI_H__
#define __LAKEAPI_H__

#include <engine.h>
#include <router.h>

namespace rebaapi
{
void initGlobal(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global);
namespace worker_group
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);
v8::Local<v8::FunctionTemplate> functionTemplate(v8::Isolate *isolate);
} // namespace worker_group
namespace router
{
extern reba::Router g_default_router;

v8::Local<v8::ObjectTemplate> objectTemplate(v8::Isolate *isolate);
void addHost(const v8::FunctionCallbackInfo<v8::Value> &args);
void removeHost(const v8::FunctionCallbackInfo<v8::Value> &args);
}
} // namespace webapi

#endif