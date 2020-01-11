# Lake
Lake is a Service Worker subset implementation for running on server side.

## Features
* Hot reconfiguration by HTTP API: Privileged Isolate to manage WorkerGroups
* Integrated cache system (?)

## Plan
* Boost Beast HTTP/1.1 Web Server
* Web Fetch API
* Websocket support
* Web Crypto API
* Basic functions: setTimeout, setInterval

## Design
Lake has two essential entities:
* WorkerGroup: A group of threads running the same script. It scales itself automatically based on the group's load.
* Router: A global instance storing the data needed to route an web request to an specific WorkerGroup

There is a special WorkerGroup created when Lake is started
* WorkerGroup Zero: It has a privileged context able to create new WorkerGroups and set routes.

The expected behavior of WorkerGroup Zero is:

* Connect to a control plane by Fetch API or Websocket
* Get the configured scripts and routes from the control plane
* Create the needed WorkerGroups
* Poll/wait for changes from the control plane


## References
* https://github.com/w3c/ServiceWorker/blob/master/implementation_considerations.md
