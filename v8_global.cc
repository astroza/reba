#include <v8_global.h>

V8Global::V8Global()
{
    v8::V8::InitializeICUDefaultLocation("lake");
    v8::V8::InitializeExternalStartupData("lake");
    this->platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(this->platform.get());
    v8::V8::Initialize();
    this->create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

V8Global::~V8Global()
{
    delete create_params.array_buffer_allocator;
}
