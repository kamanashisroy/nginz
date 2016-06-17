
HTTP module has the following constructs.

- `http\_connection` : It holds connection data. 
- `http\_hooks` : 

Diagrams
========

Here the master process accepts the user tcp connection(request level). It then sends the socket to the worker(background) process.

![http acecpt](https://cloud.githubusercontent.com/assets/973414/15868199/824f41e2-2c9b-11e6-8b40-eea0bec8be0b.jpg)

The diagram below shows how the worker(task level) process responds user request.

![http respond](https://cloud.githubusercontent.com/assets/973414/15868500/f2fd194a-2c9c-11e6-8665-5b1827a4b1a0.jpg)

The diagrams above are created by [Quick Sequence Diagram editor](http://sdedit.sourceforge.net/). Please see [the sources](sequence_diagram_http_accept.sd) and [more](sequence_diagram_http_respond.sd) for these images.

Links
======

[Quick Sequence Diagram Editor](http://sdedit.sourceforge.net/)


