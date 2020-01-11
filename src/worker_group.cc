#include <worker.h>
#include <worker_group.h>

#include <iostream>
WorkerGroup::WorkerGroup(std::string script_source, bool privileged) : script_source(script_source), privileged(privileged)
{
    add_thread((boost::thread *)new Worker(this));
}