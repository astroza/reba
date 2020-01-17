console.log("* Fiberware Lake governator loaded");
var groups = [];

var worker_script = "addEventListener('fetch', () => console.log('Test'));";
var group = new WorkerGroup(worker_script);
router.addHost('localhost:8080', group);
