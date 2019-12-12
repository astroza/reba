#include <v8_global.h>

void V8Global::V8Global()
{
    v8::V8::InitializeICUDefaultLocation("lake");
    v8::V8::InitializeExternalStartupData("lake");
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform);
    v8::V8::Initialize();
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

void ~V8Global::V8Global()
{
    delete create_params.array_buffer_allocator;
    delete platform;
}
