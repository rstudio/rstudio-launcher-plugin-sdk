cmake_minimum_required(VERSION 3.14)

project("Python QuickStart Plugin")

set(CMAKE_CXX_STANDARD 11)

include_directories(${RLPS_INCLUDE_DIR})

set(pybind11_DIR "${PYTHON_LIBRARY_DIR}/pybind11/share/cmake/pybind11")
find_package(pybind11 REQUIRED)
pybind11_add_module(rlpswrapper "bindings.cpp")


target_link_libraries(rlpswrapper PUBLIC rstudio-launcher-plugin-sdk-lib ${RLPS_BOOST_LIBS})

install(TARGETS rlpswrapper
    COMPONENT python
    LIBRARY DESTINATION "${PYTHON_LIBRARY_DIR}")
