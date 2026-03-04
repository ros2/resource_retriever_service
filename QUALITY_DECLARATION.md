This document is a declaration of software quality for the `resource_retriever_service` package, based on the guidelines in [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst).

# `resource_retriever_service` Quality Declaration

The package `resource_retriever_service` claims to be in the **Quality Level 2** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Quality Categories in REP-2004](https://reps.openrobotics.org/rep-2004/) of the ROS2 developer guide.

## Version Policy [1]

### Version Scheme [1.i]
`resource_retriever_service` uses `semver` according to the recommendation for ROS Core packages in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#versioning)

### Version Stability [1.ii]

`resource_retriever_service` is at a stable version, i.e. `>= 1.0.0`.
The current version can be found in its [package.xml](package.xml), and its change history can be found in its [CHANGELOG](CHANGELOG.rst).

### Public API Declaration [1.iii]
All symbols in the installed headers are considered part of the public API.

All installed headers are in the `include` directory of the package, headers in any other folders are not installed and considered private.

Additionally, there are generated header files which are installed and therefore part of the public API.
The source templates for these generated headers are in the `resource` folder.

### API Stability Policy [1.iv]

`resource_retriever_service` will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Policy [1.v]

`resource_retriever_service` contains C code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.

### ABI and ABI Stability Within a Released ROS Distribution [1.vi]

`resource_retriever_service` will not break API nor ABI within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

## Change Control Process [2]

`resource_retriever_service` follows the recommended guidelines for ROS Core packages in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#change-control-process).

### Change Requests [2.i]
All changes will occur through a pull request, check [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#change-control-process) for additional information.

### Contributor Origin [2.ii]
We don't have any extra measure above what is used within the github pull requests

### Peer Review Policy [2.iii]
All pull request will be peer-reviewed, check [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#change-control-process) for additional information.

### Continuous Integration [2.iv]

All pull request must pass CI on all [tier 1 platforms](https://www.ros.org/reps/rep-2000.html#support-tiers)

Currently nightly results can be seen here:
* [linux-aarch64_release](https://ci.ros2.org/view/nightly/job/nightly_linux-aarch64_release/lastBuild/testReport/resource_retriever_service/)
* [linux_release](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/resource_retriever_service/)
* [mac_osx_release](https://ci.ros2.org/view/nightly/job/nightly_osx_release/lastBuild/testReport/resource_retriever_service/)
* [windows_release](https://ci.ros2.org/view/nightly/job/nightly_win_rel/lastBuild/testReport/resource_retriever_service/)

###  Documentation Policy [2.v]

All pull requests must resolve related documentation changes before merging

## Documentation [3]

### Feature Documentation [3.i]

`resource_retriever_service` has a documented feature list and it is hosted [here](http://docs.ros2.org/latest/api/resource_retriever_service/index.html).

### Public API Documentation [3.ii]

`resource_retriever_service` has documentation of its public API within the header files of the relevant APIs.

### License [3.iii]

The license for `resource_retriever_service` is Apache 2.0 and BSD, and a summary is in each source file, the type is declared in the package.xml of the related sub-package.

The interfaces and service packages are licensed under Apache 2.0 while the plugin is currently licensed under BDS, each sub package has its relevant LICENSE file and the source files are marked acordingly.

### Copyright Statements [3.iv]

The copyright holders each provide a statement of copyright in each source code file in `resource_retriever_service`.

## Testing [4]

### Feature Testing [4.i]

Each feature in `resource_retriever_service` has corresponding tests which simulate typical usage, and they are located in the [`test`](https://github.com/ros2/resource_retriever_service/tree/main/test) directory.
New features are required to have tests before being added.
Currently nightly test results can be seen here:
* [linux-aarch64_release](https://ci.ros2.org/view/nightly/job/nightly_linux-aarch64_release/lastBuild/testReport/resource_retriever_service/)
* [linux-arm64_release](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/resource_retriever_service/)
* [mac_osx_release](https://ci.ros2.org/view/nightly/job/nightly_osx_release/lastBuild/testReport/resource_retriever_service/)
* [windows_release](https://ci.ros2.org/view/nightly/job/nightly_win_rel/lastBuild/testReport/resource_retriever_service/)

### Public API Testing [4.ii]

Each part of the public API has tests, and new additions or changes to the public API require tests before being added.
The tests aim to cover both typical usage and corner cases, but are quantified by contributing to code coverage.

### Coverage [4.iii]

`resource_retriever_service` follows the recommendations for ROS Core packages in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#code-coverage), and opts to use line coverage instead of branch coverage.

This includes:

- tracking and reporting line coverage statistics
- achieving and maintaining a reasonable branch line coverage (90-100%)
- no lines are manually skipped in coverage calculations

Changes are required to make a best effort to keep or increase coverage before being accepted, but decreases are allowed if properly justified and accepted by reviewers.

Current coverage statistics can be viewed [here](https://ci.ros2.org/job/ci_linux_coverage/lastSuccessfulBuild/cobertura/src_ros2_resource_retriever_service_src/).
A description of how coverage statistics are summarized from this page, can be found in the ["ROS 2 Onboarding Guide"](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#note-on-coverage-runs).

### Performance [4.iv]

`resource_retriever_service` follows the recommendations for performance testing of C code in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#performance), and opts to do performance analysis on each release rather than each change.
It is not yet defined if this package requires performance testing and how addresses this topic.

### Linters and Static Analysis [4.v]

`resource_retriever_service` uses and passes all the ROS2 standard linters and static analysis tools for a C package as described in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/The-ROS2-Project/Contributing/Developer-Guide.html#linters-and-static-analysis). Passing implies there are no linter/static errors when testing against CI of supported platforms.

Currently nightly test results can be seen here:
* [linux-aarch64_release](https://ci.ros2.org/view/nightly/job/nightly_linux-aarch64_release/lastBuild/testReport/resource_retriever_service/)
* [linux-arm64_release](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/resource_retriever_service/)
* [mac_osx_release](https://ci.ros2.org/view/nightly/job/nightly_osx_release/lastBuild/testReport/resource_retriever_service/)
* [windows_release](https://ci.ros2.org/view/nightly/job/nightly_win_rel/lastBuild/testReport/resource_retriever_service/)

## Dependencies [5]

### Direct Runtime ROS Dependencies [5.i]
There are several direct dependencies from the subpackages on external packages. They are listed below.

 * rclcpp [Level 2 Quality]
 * resource_retriever [Level unknown]

It has several "buildtool" and "test" dependencies, which do not affect the resulting quality of the package, because they do not contribute to the public library API.

### Optional Direct Runtime ROS Dependencies [5.ii]
`resource_retriever_service` has no run-time or build-time dependencies that need to be considered for this declaration.

### Direct Runtime non-ROS Dependency [5.iii]
`resource_retriever_service` has no run-time or build-time dependencies that need to be considered for this declaration.

## Platform Support [6]

`resource_retriever_service` supports all of the tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers), and tests each change against all of them.

Currently nightly build status can be seen here:
* [linux-aarch64_release](https://ci.ros2.org/view/nightly/job/nightly_linux-aarch64_release/lastBuild/resource_retriever_service/)
* [linux-arm64_release](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/resource_retriever_service/)
* [mac_osx_release](https://ci.ros2.org/view/nightly/job/nightly_osx_release/lastBuild/resource_retriever_service/)
* [windows_release](https://ci.ros2.org/view/nightly/job/nightly_win_rel/lastBuild/resource_retriever_service/)

## Vulnerability Disclosure Policy [7.i]

This package conforms to the Vulnerability Disclosure Policy in [REP-2006](https://www.ros.org/reps/rep-2006.html).

# Current status Summary

The chart below compares the requirements in the REP-2004 with the current state of the resource_retriever_service package.
|Number|  Requirement| Current state |
|--|--|--|
|1| **Version policy** |---|
|1.i|Version Policy available | ✓ |
|1.ii|Stable version |✓||
|1.iii|Declared public API|✓|
|1.iv|API stability policy|✓|
|1.v|ABI stability policy|✓|
|1.vi_|API/ABI stable within ros distribution|✓|
|2| **Change control process** |---|
|2.i| All changes occur on change request | ✓|
|2.ii| Contributor origin (DCO, CLA, etc) | ✓|
|2.iii| Peer review policy | ✓ |
|2.iv| CI policy for change requests | ✓ |
|2.v| Documentation policy for change requests | ✓ |
|3| **Documentation** | --- |
|3.i| Per feature documentation | ✓ |
|3.ii| Per public API item documentation | ✓ |
|3.iii| Declared License(s) | ✓ |
|3.iv| Copyright in source files| ✓ |
|3.v.a| Quality declaration linked to README | ✓ |
|3.v.b| Centralized declaration available for peer review |✓|
|4| Testing | --- |
|4.i| Feature items tests | ✓ |
|4.ii| Public API tests | ✓ |
|4.iii.a| Using coverage |✓ |
|4.iii.a| Coverage policy | ✓ |
|4.iv.a| Performance tests (if applicable) | ☓ |
|4.iv.b| Performance tests policy| ✓ |
|4.v.a| Code style enforcement (linters)| ✓ |
|4.v.b| Use of static analysis tools | ✓ |
|5| Dependencies | --- |
|5.i| Must not have ROS lower level dependencies | ✓ |
|5.ii| Optional ROS lower level dependencies| ✓ |
|5.iii| Justifies quality use of non-ROS dependencies |✓|
|6| Platform support | --- |
|6.i| Support targets Tier1 ROS platforms| ✓ |
|7| Security | --- |
|7.i| Vulnerability Disclosure Policy | ✓ |