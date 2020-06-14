#include <worker.h>
#include <worker_group.h>
#include <boost/asio.hpp>

#include <iostream>
namespace lake
{
    WorkerGroup::WorkerGroup(std::string script_source,
                             bool privileged) : script_source(script_source), privileged(privileged)
    {
        if (privileged)
        {
            scale_up();
        }
    }

    void WorkerGroup::add_worker(Worker *new_worker)
    {
        workers_.push_back(new_worker);
    }

    Worker *WorkerGroup::scale_up()
    {
        auto new_worker = new Worker(this);
        add_worker(new_worker);
        return new_worker;
    }

    size_t WorkerGroup::size()
    {
        return workers_.size();
    }

    void WorkerGroup::delegate_request()
    {
        Worker *selected_worker;
        if (size() > 0)
        {
            auto it = workers_.begin();
            selected_worker = *it;
        }
        else
        {
            selected_worker = scale_up();
        }
        boost::asio::post(selected_worker->io_context, [selected_worker]() {
            selected_worker->process_request();
        });
    }
} // namespace lake