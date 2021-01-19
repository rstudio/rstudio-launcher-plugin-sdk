RStudio Launcher Plugin SDK Smoke Test Tool
===========================================

The smoke test tool that can be built from this project is a manual command line utility for 
exercising the basic code paths of each request that the Plugin may receive. 

Usage
-----
If the Plugin correctly supports the `unprivileged=1` option, it is possible to run the smoke test 
tool as any user. If the smoke test tool is not run as root, the user running the smoke test tool 
and the user passed to the smoke test tool as a parameter must be the same. Additionally, the 
Plugin should be configured so that the scratch path is read-and-write-accessible to the same user.

To run the tool:
```
[sudo ]<path/to/cmake-build-dir/smoke-test>/rlps-smoke-test <path/to/plguin/cmake-build-dir/plugin-name> <user>
```

On startup, the smoke test tool will attempt to bootstrap the Plugin. On success, it will display a 
menu of actions which may be executed by entering the matching number and pressing `ENTER`. Except 
for the `Exit` action, each action will send a request to the Plugin and then print the response(s) 
that it receives before displaying the menu again. If the Plugin does not return a response within 
30 seconds of the request time, the smoke test tool will emit an error message and then exit.

Caveats
-------
The smoke test tool does not test all required functionality for each request type. It is meant as a 
utility to help ensure that the Plugin is behaving as expected in the basic case, and to facilitate 
debugging requests. To ensure that the Plugin is fully functional, it should be tested against 
RStudio Workbench with the RStudio Launcher enabled.
