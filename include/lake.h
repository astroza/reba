#ifndef __LAKE_H__
#define __LAKE_H__

#include <v8_global.h>

namespace lake
{
v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged);
}

#endif