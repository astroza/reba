#include <boost/asio.hpp>
#include <webapi.h>
#include <worker.h>


namespace webapi
{
namespace timer
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    if (!args.IsConstructCall()) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Function is a constructor", v8::NewStringType::kNormal).ToLocalChecked());
        return;
    }
    Worker *worker = static_cast<Worker *>(isolate->GetData(0));
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value script(isolate, args[0]);
    auto timer = new boost::asio::deadline_timer(worker->io_context);
    timer->expires_from_now(boost::posix_time::milliseconds(5000));
    timer->async_wait([](const boost::system::error_code& ec)
    {
        puts("Timer!");
    });
    new lake::NativeBind(isolate, args.This(), timer, lake::NativeBindDeleteCallback<boost::asio::deadline_timer>);
}

v8::Local<v8::FunctionTemplate> function_template(v8::Isolate *isolate)
{
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> func_tmpl = v8::FunctionTemplate::New(isolate, constructor);
    func_tmpl->SetClassName(v8::String::NewFromUtf8(isolate, "Timer", v8::NewStringType::kNormal).ToLocalChecked());
    func_tmpl->InstanceTemplate()->SetInternalFieldCount(1); // For native bind
    return handle_scope.Escape(func_tmpl);
}
} // namespace timer
} // namespace webapi