#ifndef __WEBAPI_H__
#define __WEBAPI_H__

#include <lake.h>
#include <src/base/platform/time.h>
#include <src/debug/debug-interface.h>
#include <src/debug/interface-types.h>

namespace webapi
{
class Console : public v8::debug::ConsoleDelegate
{
public:
	explicit Console(v8::Isolate *isolate);

private:
	void Assert(const v8::debug::ConsoleCallArguments &args,
				const v8::debug::ConsoleContext &) override;
	void Log(const v8::debug::ConsoleCallArguments &args,
			 const v8::debug::ConsoleContext &) override;
	void Error(const v8::debug::ConsoleCallArguments &args,
			   const v8::debug::ConsoleContext &) override;
	void Warn(const v8::debug::ConsoleCallArguments &args,
			  const v8::debug::ConsoleContext &) override;
	void Info(const v8::debug::ConsoleCallArguments &args,
			  const v8::debug::ConsoleContext &) override;
	void Debug(const v8::debug::ConsoleCallArguments &args,
			   const v8::debug::ConsoleContext &) override;
	void Time(const v8::debug::ConsoleCallArguments &args,
			  const v8::debug::ConsoleContext &) override;
	void TimeEnd(const v8::debug::ConsoleCallArguments &args,
				 const v8::debug::ConsoleContext &) override;
	void TimeStamp(const v8::debug::ConsoleCallArguments &args,
				   const v8::debug::ConsoleContext &) override;
	void Trace(const v8::debug::ConsoleCallArguments &args,
			   const v8::debug::ConsoleContext &) override;
	v8::Isolate *isolate;
	std::map<std::string, v8::base::TimeTicks> timers;
	v8::base::TimeTicks default_timer;
};
namespace timer
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);
v8::Local<v8::FunctionTemplate> function_template(v8::Isolate *isolate);
void set_timeout(const v8::FunctionCallbackInfo<v8::Value> &args);
} // namespace timer
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global);
} // namespace webapi

#endif