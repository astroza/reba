#include <webapi.h>
#include <worker.h>

// Should I cache v8::string constants ? as seen as internal isolate->factory()->InternalizeUtf8String ?
// 0:) Yes, optimize everything
// >:| No, addEventListener is a low frequency call
namespace webapi {
namespace event {
    static std::unordered_map<std::string_view, int> eventMap({ { "fetch", reba::WorkerCallbackIndex::Value::FetchEvent } });

    void addEventListener(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        auto isolate = args.GetIsolate();
        auto context = isolate->GetCurrentContext();
        v8::HandleScope handle_scope(isolate);
        if (args.Length() < 2) {
            auto exception_msg = v8::String::NewFromUtf8(isolate, "Insufficient arguments", v8::NewStringType::kNormal);
            isolate->ThrowException(exception_msg.ToLocalChecked());
            return;
        }
        if (!args[0]->IsString()) {
            auto exception_msg = v8::String::NewFromUtf8(isolate, "First argument must be a string", v8::NewStringType::kNormal);
            isolate->ThrowException(exception_msg.ToLocalChecked());
            return;
        }
        if (args[1]->IsFunction()) {
            v8::String::Utf8Value event_type(isolate, args[0]->ToString(context).ToLocalChecked());
            auto found = eventMap.find(std::string_view(*event_type));
            if (found != eventMap.end()) {
                auto worker = static_cast<reba::Worker*>(isolate->GetData(reba::IsolateDataIndex::Value::Worker));
                auto event_callback = v8::Local<v8::Function>::Cast(args[1]);
                // TODO: change from setCallback to addCallback
                worker->setCallback(static_cast<reba::WorkerCallbackIndex::Value>(found->second), event_callback);
            }
        }
    }
} // namespace event
} // namespace webapi