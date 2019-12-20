#include <v8.h>
#include <libplatform/libplatform.h>

struct V8Global {
    std::unique_ptr<v8::Platform> platform;
    v8::Isolate::CreateParams create_params;

    V8Global();
    ~V8Global();
};