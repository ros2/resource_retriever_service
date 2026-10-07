# resource_retriever_service
A resource retriever service definition, plugin, and in memory service implementation to allow rviz to use remote mesh files for visualization

# Configuration

## Service call timeout

The plugin (`resource_retriever_service_plugin::RosServiceResourceRetriever`) waits a limited time for the service to answer each resource request.
The limit is the integer parameter `resource_retriever_service_timeout_ms` of the node the plugin was created with, for example the RViz node:

 * Unit: milliseconds.
 * Default: `3000`.
 * Valid range: `1` to `86400000` (24 hours).

Large resources on slow networks may need a larger value:

```bash
ros2 run rviz2 rviz2 --ros-args -p resource_retriever_service_timeout_ms:=30000
```

An initial value outside of the valid range is ignored with a warning, and the default is used instead.

The parameter is declared when the first plugin instance is created with the node, and it is shared by all the plugin instances created with that node.
It can be changed at runtime, and the new value applies to the requests made afterwards:

```bash
ros2 param set <node_name> resource_retriever_service_timeout_ms 30000
```

A request blocks the caller until the service answers or the timeout expires, so in RViz a larger value also means a longer freeze when a server stops answering.

# Quality Declarations.
The quality declarations for the sub packages can be found in their respective folders.
 * [Interfaces](resource_retriever_interfaces/QUALITY_DECLARATION.md)
 * [Service](resource_retriever_service/QUALITY_DECLARATION.md)
 * [Plugin](resource_retriever_service_plugin/QUALITY_DECLARATION.md)
