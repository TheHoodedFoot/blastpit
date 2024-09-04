var factory = require('./ems_node_generated.js');

factory().then((instance) => {
  instance._sayHi(); // direct calling works
  instance.ccall("sayHi"); // using ccall etc. also work
  console.log(instance._daysInWeek()); // values can be returned, etc.
});
