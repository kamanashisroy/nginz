
Parallel processing
===================

The parallelism achieved by this module is process based rather than thread-based. So there is no memory sharing. The processes communicate with each other through sockets.

This module facilitates parallel communication over processes. The processes form a line topology. They communicate over unix socket by token passing.

Note that the design is event based. So all the actions start from the `event_loop.c`. Eventually the `event_loop.c` is executed by the `fiber.c` which facilitates co-operative non-preemptive multitasking.

### Master and Worker

The network stacks are implemented by master listener process and child workers. The child workers load-balance the connections among them. The socket connections literally jumps from one worker process to another. This jumping is supported by linux implementation of [sendmsg and recvmsg](http://linux.die.net/man/2/sendmsg). 

#### Serving an http request scenario ..

The table below shows the socket/connection jumping over the worker processes. The master process listens for new connection by [`event_loop.c`:`event_loop_register_fd`](../event_loop.c) . When there is a new connection `event_loop.c` triggers the event callback [`tcp_listener.c:on_connection()`](../net/tcp_listener.c). 


| Master | Worker 1 | Worker 2 | Worker 3 | More ..
| --- | --- | --- | --- |--- 
| New TCP Connection:<br>`tcp_listener.c:on_connection()`<br>calls `stack.on_tcp_connection(fd)`<br>=`http_accept.c:http_on_tcp_connection(fd)`<br>calls `pipeline.c:pp_bubble_down_send_socket(fd,msg)`<br>calls `sendmsg` | | | | 
| | `pipeline.c:on_bubble_down_recv_socket()`<br>load balance `skip_load()`<br>calls `pipeline.c:pp_bubble_down_send_socket(fd,msg)`| | | 
| ... | ... | ... | ... | ...


The [`tcp_listener.c`](../net/tcp_listener.c) resolves the associated protocol dynamically. It eventually calls the [`tcp_listener.c:http_on_tcp_connection(fd)`](../net/http/http_accept.c) with newly accepted TCP client.

The http module sends the connection/socket to the worker process. It does it by calling [`pipeline.c:pp_bubble_down_send_socket(fd,msg)`](pipeline.c).

On the other end the worker process gets notification. It does the load-balancing.

```C
// simple load balancing code
static int skipped = 0;
static int skip_load() {
        int load = event_loop_fd_count();
        load = load + skipped;
        skipped++;
        return (load % (NGINZ_NUMBER_OF_PROCESSORS-1));
}
```

Then it can skip the task and pass onto the child worker process. It can also notify the http module to process the data. It takes the decision based on the above load-balancing code.

| Master | Worker 1 | Worker 2 | Worker 3 | More ..
| --- | --- | --- | --- |--- 
| New TCP Connection:<br>`tcp_listener.c:on_connection()`<br>calls `stack.on_tcp_connection(fd)`<br>=`http_accept.c:http_on_tcp_connection(fd)`<br>calls `pipeline.c:pp_bubble_down_send_socket(fd,msg)`<br>calls `sendmsg` | | | | 
| | `pipeline.c:on_bubble_down_recv_socket()`<br>calls `stack.on_connection_bubble(fd,msg)`<br>=`http_accept.c:http_on_connection_bubble(fd,msg)`<br>calls `event_loop.c:event_loop_register_fd()` | | | 
| ... | ... | ... | ... | ...


In the scenario above the `worker 1` processes user request.

