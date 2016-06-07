
Plugin and dependency injection
===================================

The plugin feature gives the ability to change/vary the parts. It has two key features,

- It allows decomposition of a subsystem into individual modules.
- It has a `plugin-space`, which is a name-abstraction. And the name connects the modules to enable a integrated service. 

So plugin gives the **naming tool** for communication between the modules. Each `plugin-space` is a name. It is called the `plugin-space`. Each module can register at multiple `plugin-space`.

Plugin-system literally allow to grow the framework in small modules. The modules can have hierarchy(subsystems) and layers(closely linked to a subsystem with packaging).

Hierarchy of nginz
====================

The hierarchy of the core modules are given below. The blue-filled boxes are the libraryes. And green-filled boxes are modules.

![hierarchy](https://cloud.githubusercontent.com/assets/973414/15866119/3b56bbf2-2c92-11e6-8023-5bbf0025428c.png)

It is notable that the modules has least dependency among each other. So it restricts the propagation of error. It also supports cohesion in the contrary of the dependency-hell. And as a result of that, it makes the development and testing easier.

Design decisions
================

There are several design dicisions made to make it simple. Some of them are,

- In addition to static loading, it allows dynamic loading of plugins. And it allows loading the module that is now (fully)useful right now(but will be when other modules are available). 
- It does not need to include header files(because, it needs the cognitive load of knowing the API). 

One of the draw-back of (no/limited)-header design, is that the programmer need to see the documentation/example or even the implementation to find the plugin-points. It is suggested that the modules write documents and examples of the use of their `plugs` and `points`.

Plugin-table
===========================

| shortname | target | plugin-space | related source 
| --- | --- | --- | --- 
| plugin |	shake |	shake/plugin |	src/plugin_manager.c
| ||| 		It dumps the avilable plugins
| pipeline |	fork |	fork/child/after |	src/parallel/pipeline.c
| ||| 		It allows the processes to pipeline messages to and forth.
| shake |	fork |	fork/child/after |	src/shake.c
| ||| 		It stops control socket in child process
| pipeline |	fork |	fork/parent/after |	src/parallel/pipeline.c
| ||| 		It allows the processes to pipeline messages to and forth.
| ping |	shake |	shake/ping |	src/parallel/ping.c
| ||| 		It pings the child process. for example(ping quit) will quit all the child process.
| quitall |	shake |	shake/quitall |	src/shake/quitall.c
| ||| 		It stops the fibers and sends quitall msg to other processes.
| quitall |	shake |	shake/softquitall |	src/shake/quitall.c
| ||| 		It stops the fibers and sends quitall msg to other processes.
| binary_coder_test |	test |	test/binary_coder_test |	src/binary_coder_extended.c
| ||| 		It is test code for binary coder.
| fiber |	shake |	shake/fiber |	src/fiber.c
| ||| 		It will show the number of active fibers.
| test |	shake |	shake/test |	src/shake/test.c
| ||| 		It tests all the test cases.


Sources
========

- [plugin.c](src/plugin.c)
- [plugin_manager.c](src/plugin_manager.c)

