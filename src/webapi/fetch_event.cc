#include <webapi.h>
#include <worker.h>

// Should I cache v8::string constants ? as seen as internal isolate->factory()->InternalizeUtf8String ?
// 0:) Yes, optimize everything
// >:| No, addEventListener is a low frequency call
namespace webapi
{
namespace event
{
void addEventListener(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::HandleScope handle_scope(isolate);
    if (args.Length() < 2)
    {
        auto exception_msg = v8::String::NewFromUtf8(isolate, "Insufficient arguments", v8::NewStringType::kNormal);
        isolate->ThrowException(exception_msg.ToLocalChecked());
        return;
    }
    if (!args[0]->IsString())
    {
        auto exception_msg = v8::String::NewFromUtf8(isolate, "First argument must be a string", v8::NewStringType::kNormal);
        isolate->ThrowException(exception_msg.ToLocalChecked());
        return;
    }
    if (args[1]->IsFunction())
    {
        // Is it a fetch listener ?
        auto fetch_string = v8::String::NewFromUtf8(isolate, "fetch", v8::NewStringType::kNormal);
        if (args[0]->Equals(context, fetch_string.ToLocalChecked()).FromJust())
        {
            auto worker = static_cast<reba::Worker *>(isolate->GetData(reba::IsolateDataIndex::Value::Worker));
            auto fetch_callback = v8::Local<v8::Function>::Cast(args[1]);
            worker->setCallback(reba::WorkerCallbackIndex::Value::FetchEvent, fetch_callback);
        }
    }
}
} // namespace event
} // namespace webapi