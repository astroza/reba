#ifndef __WEBAPI_H__
#define __WEBAPI_H__

#include <lake.h>

namespace webapi
{
namespace console
{
void log(const v8::FunctionCallbackInfo<v8::Value> &args);
} // namespace console
namespace timer
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);
v8::Local<v8::FunctionTemplate> function_template(v8::Isolate *isolate);
} // namespace timer
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global);
} // namespace webapi

#endif