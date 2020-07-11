#include <worker.h>
#include <worker_group.h>
#include <boost/asio.hpp>

#include <iostream>
namespace reba
{
    WorkerGroup::WorkerGroup(std::string script_source,
                             bool privileged) : script_source(script_source), privileged(privileged)
    {
        if (privileged)
        {
            CreateWorker();
        }
        it = workers_.begin();
    }

    void WorkerGroup::RegisterWorker(Worker *new_worker)
    {
        workers_.push_back(new_worker);
    }

    Worker *WorkerGroup::CreateWorker()
    {
        Worker *ret;
        int count = this->privileged? 1 : 8;
        for(int i = 0; i < count; i++) {
            auto new_worker = new Worker(this);
            RegisterWorker(new_worker);
            ret = new_worker;
        }
        return ret;
    }

    size_t WorkerGroup::Size()
    {
        return workers_.size();
    }

    Worker *WorkerGroup::SelectOrCreateWorker() {
        Worker *selected_worker;
        
        if (Size() > 0)
        {
            if(it == workers_.end()) {
                it = workers_.begin();
            }
            selected_worker = *it;
            it++;
        }
        else
        {
            selected_worker = CreateWorker();
        }
        return selected_worker;
    }
} // namespace reba