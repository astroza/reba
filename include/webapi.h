#ifndef __WEBAPI_H__
#define __WEBAPI_H__

#include <v8_global.h>

namespace webapi
{
namespace console
{
void log(const v8::FunctionCallbackInfo<v8::Value> &args);
} // namespace console
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global);
} // namespace webapi

#endif