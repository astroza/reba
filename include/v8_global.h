#include <include/v8.h>
#include <include/libplatform/libplatform.h>

struct V8Global {
    v8::Platform *platform;
    v8::Isolate::CreateParams create_params;

    V8Global();
    ~V8Global();
};