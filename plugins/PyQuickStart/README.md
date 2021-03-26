## About

This is a sample of how pybind11 can be used to implement a Plugin using the SDK in python. Because of limitations with the binding interface, private virtual methods (pure or not) cannot be exposed to python. As a result, any private virtual methods that you wish to override in your plugin will have to be moved from the private section to the protected section in the HPP file. For example, that was already done in AbstractMain.hpp.

In bindings.cpp, an `AbstractMainTrampoline` class was added to inform python which methods are overridable, and whether they are pure virtual (no implementation) or not. Additionally, `AbstractMainPublisher` was added to update the visibility of the virtual methods from protected to public, so that the python binding library can access them. It is not possible to do this with private methods, which is why they need to be moved. Alternatively, you may wish to simply move the private methods into the public section and skip the Publisher altogether. You can read more about Trampolines and Publishers in [this section of the pybind11 docs](https://pybind11.readthedocs.io/en/stable/advanced/classes.html#).

Besides AbstractMain, all classes should be bound to python with the `std::shared_ptr` storage type, as shared pointers are used internally. This can be done like this: `pybind11::class_<ClassName, std::shared_ptr<ClassName> >(m, "ClassName").def(...)`.

Perhaps a simpler solution, which I haven't experimented with, would be to implement the guts of the plugin in python and then use `pybind11` in the other direction to invoke those methods from the C++ QuickStart Plugin method.

## Compiling and Running:

1. [Install pybind11](https://pybind11.readthedocs.io/en/stable/installing.html) as desired .
2. From the root of the project, configure cmake:
   <ol type="a">
    <li><code>mkdir -p build/</code></li>
    <li><code>cd build/</code></li>
    <li><code>cmake .. -DPYTHON_LIBRARY_DIR=/path/to/site-packages/ -DPYTHON_EXECUTABLE=/path/to/python -G "Unix Makefiles"</code></li>
   </ol>
3. Compile and install the Cpython library (from the `build` directory):
   <ol type="a">
    <li><code>make -j4</code> (Note: 4 is the number of threads to use when compiling. More or less may be used).</li>
    <li><code>make install</code></li>
   </ol>
4. The `QuickStartMain.py` file may now be used as any other plugin binary.
