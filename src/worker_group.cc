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
            createWorker();
        }
        it = workers_.begin();
    }

    void WorkerGroup::registerWorker(Worker *new_worker)
    {
        workers_.push_back(new_worker);
    }

    Worker *WorkerGroup::createWorker()
    {
        Worker *ret;
        int count = this->privileged? 1 : 8;
        for(int i = 0; i < count; i++) {
            auto new_worker = new Worker(this);
            registerWorker(new_worker);
            ret = new_worker;
        }
        return ret;
    }

    size_t WorkerGroup::size()
    {
        return workers_.size();
    }

    Worker *WorkerGroup::selectOrCreateWorker() {
        Worker *selected_worker;
        
        if (size() > 0)
        {
            if(it == workers_.end()) {
                it = workers_.begin();
            }
            selected_worker = *it;
            it++;
        }
        else
        {
            selected_worker = createWorker();
        }
        return selected_worker;
    }
} // namespace reba