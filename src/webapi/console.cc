#include <iostream>
#include <webapi.h>

namespace webapi
{
namespace console
{
void log(const v8::FunctionCallbackInfo<v8::Value> &args)
{
	v8::Isolate *isolate = args.GetIsolate();
	for (int i = 0; i < args.Length(); i++)
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		v8::String::Utf8Value str(args.GetIsolate(), args[i]);
		if (*str)
		{
			std::cout << *str;
		}
	}
	std::cout << std::endl;
}
} // namespace console
} // namespace webapi