#ifndef __LAKEAPI_H__
#define __LAKEAPI_H__

#include <v8_global.h>

namespace lakeapi
{
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global);
} // namespace webapi

#endif