
Plugin and dependency injection
===================================

Plugin is the **naming tool** for communication between the modules. Each plugin is addressed by the name. It is called the `plugin-space`. Each module can register at multiple `plugin-space`.

Plugin-system literally allow to grow the framework in small modules. The modules can have hierarchy(subsystems) and layers(closely linked to a subsystem with packaging).

Hierarchy of nginz
====================

The hierarchy of the core modules are given below. The blue-filled boxes are the libraryes. And green-filled boxes are modules.

![hierarchy](subsystem.dot)

It is notable that the modules has least dependency among each other. So it restricts the propagation of error. It also supports cohesion in the contrary of the dependency-hell. And as a result of that, it makes the development and testing easier.


TODO dump a plugin-table
===========================


Sources
========
[plugin.c](plugin.c)
[plugin_manager.c](plugin_manager.c)

