#include <iostream>
#include <webapi.h>

#include <src/execution/isolate.h>

static void WriteToFile(const char *prefix, FILE *file, v8::Isolate *isolate,
						const v8::debug::ConsoleCallArguments &args)
{
	if (prefix)
	{
		fprintf(file, "%s: ", prefix);
	}
	for (int i = 0; i < args.Length(); i++)
	{
		v8::HandleScope handle_scope(isolate);
		if (i > 0)
			fprintf(file, " ");

		v8::Local<v8::Value> arg = args[i];
		v8::Local<v8::String> str_obj;

		if (arg->IsSymbol())
		{
			arg = v8::Local<v8::Symbol>::Cast(arg)->Description();
		}
		if (!arg->ToString(isolate->GetCurrentContext()).ToLocal(&str_obj))
		{
			return;
		}
		v8::String::Utf8Value str(isolate, str_obj);
		int n = static_cast<int>(fwrite(*str, sizeof(**str), str.length(), file));
		if (n != str.length())
		{
			printf("Error in fwrite\n");
			v8::base::OS::ExitProcess(1);
		}
	}
	fprintf(file, "\n");
}

namespace webapi
{
Console::Console(v8::Isolate *isolate) : isolate(isolate)
{
	default_timer = v8::base::TimeTicks::HighResolutionNow();
}

void Console::Assert(const v8::debug::ConsoleCallArguments &args,
					 const v8::debug::ConsoleContext &)
{
	// If no arguments given, the "first" argument is undefined which is
	// false-ish.
	if (args.Length() > 0 && args[0]->BooleanValue(isolate))
	{
		return;
	}
	WriteToFile("console.assert", stdout, isolate, args);
	isolate->ThrowException(v8::Exception::Error(
		v8::String::NewFromUtf8(isolate, "console.assert failed",
								v8::NewStringType::kNormal)
			.ToLocalChecked()));
}

void Console::Log(const v8::debug::ConsoleCallArguments &args,
				  const v8::debug::ConsoleContext &)
{
	WriteToFile(nullptr, stdout, isolate, args);
}

void Console::Error(const v8::debug::ConsoleCallArguments &args,
					const v8::debug::ConsoleContext &)
{
	WriteToFile("console.error", stderr, isolate, args);
}

void Console::Warn(const v8::debug::ConsoleCallArguments &args,
				   const v8::debug::ConsoleContext &)
{
	WriteToFile("console.warn", stdout, isolate, args);
}

void Console::Info(const v8::debug::ConsoleCallArguments &args,
				   const v8::debug::ConsoleContext &)
{
	WriteToFile("console.info", stdout, isolate, args);
}

void Console::Debug(const v8::debug::ConsoleCallArguments &args,
					const v8::debug::ConsoleContext &)
{
	WriteToFile("console.debug", stdout, isolate, args);
}

void Console::Time(const v8::debug::ConsoleCallArguments &args,
				   const v8::debug::ConsoleContext &)
{
	if (args.Length() == 0)
	{
		default_timer = v8::base::TimeTicks::HighResolutionNow();
	}
	else
	{
		v8::Local<v8::Value> arg = args[0];
		v8::Local<v8::String> label;
		v8::TryCatch try_catch(isolate);
		if (!arg->ToString(isolate->GetCurrentContext()).ToLocal(&label))
			return;
		v8::String::Utf8Value utf8(isolate, label);
		std::string string(*utf8);
		auto find = timers.find(string);
		if (find != timers.end())
		{
			find->second = v8::base::TimeTicks::HighResolutionNow();
		}
		else
		{
			timers.insert(std::pair<std::string, v8::base::TimeTicks>(
				string, v8::base::TimeTicks::HighResolutionNow()));
		}
	}
}

void Console::TimeEnd(const v8::debug::ConsoleCallArguments &args,
					  const v8::debug::ConsoleContext &)
{
	v8::base::TimeDelta delta;
	if (args.Length() == 0)
	{
		delta = v8::base::TimeTicks::HighResolutionNow() - default_timer;
		printf("console.timeEnd: default, %f\n", delta.InMillisecondsF());
	}
	else
	{
		v8::base::TimeTicks now = v8::base::TimeTicks::HighResolutionNow();
		v8::Local<v8::Value> arg = args[0];
		v8::Local<v8::String> label;
		v8::TryCatch try_catch(isolate);
		if (!arg->ToString(isolate->GetCurrentContext()).ToLocal(&label))
		{
			return;
		}
		v8::String::Utf8Value utf8(isolate, label);
		std::string string(*utf8);
		auto find = timers.find(string);
		if (find != timers.end())
		{
			delta = now - find->second;
		}
		printf("console.timeEnd: %s, %f\n", *utf8, delta.InMillisecondsF());
	}
}

void Console::TimeStamp(const v8::debug::ConsoleCallArguments &args,
						const v8::debug::ConsoleContext &)
{
	v8::base::TimeDelta delta = v8::base::TimeTicks::HighResolutionNow() - default_timer;
	if (args.Length() == 0)
	{
		printf("console.timeStamp: default, %f\n", delta.InMillisecondsF());
	}
	else
	{
		v8::Local<v8::Value> arg = args[0];
		v8::Local<v8::String> label;
		v8::TryCatch try_catch(isolate);
		if (!arg->ToString(isolate->GetCurrentContext()).ToLocal(&label))
			return;
		v8::String::Utf8Value utf8(isolate, label);
		std::string string(*utf8);
		printf("console.timeStamp: %s, %f\n", *utf8, delta.InMillisecondsF());
	}
}

void Console::Trace(const v8::debug::ConsoleCallArguments &args,
					const v8::debug::ConsoleContext &)
{
	v8::internal::Isolate *i_isolate = reinterpret_cast<v8::internal::Isolate *>(isolate);
	i_isolate->PrintStack(stderr, v8::internal::Isolate::kPrintStackConcise);
}
} // namespace webapi