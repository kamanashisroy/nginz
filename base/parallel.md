
Parallel processing
===================

The parallelism achieved by this module is process based rather than thread-based. So there is no memory sharing. The processes communicate with each other through sockets.

This module facilitates parallel communication over processes. The processes form a line topology. They communicate over unix socket by token passing.

Note that the design is event based. So all the actions start from the `event_loop.c`. Eventually the `event_loop.c` is executed by the `fiber.c` which facilitates co-operative non-preemptive multitasking.

### Master and Worker

The network stacks are implemented by master listener process and child workers. The child workers load-balance the connections among them. The socket connections literally jumps from one worker process to another. This jumping is supported by linux implementation of [sendmsg and recvmsg](http://linux.die.net/man/2/sendmsg). 

#### Serving an http request scenario ..

Here the master process accepts the user tcp connection(request level). It then sends the socket to the worker(background) process.

![http acecpt](https://cloud.githubusercontent.com/assets/973414/15868199/824f41e2-2c9b-11e6-8b40-eea0bec8be0b.jpg)

The diagram below shows how the worker(task level) process responds user request.

![http respond](https://cloud.githubusercontent.com/assets/973414/15868500/f2fd194a-2c9c-11e6-8665-5b1827a4b1a0.jpg)

Compare and Swap database
==========================

There is a compare and swap database available. It can be used to set a key,value pair accross all the processes. Please see the `async\_db.c` for the implementation details.


