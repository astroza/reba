#ifndef __LAKEAPI_H__
#define __LAKEAPI_H__

#include <engine.h>

namespace lakeapi
{
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global);
namespace worker_group
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);
v8::Local<v8::FunctionTemplate> function_template(v8::Isolate *isolate);
} // namespace worker_group
} // namespace webapi

#endif