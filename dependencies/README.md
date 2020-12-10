# Purpose

The scripts in this folder help new RStudio Launcher Plugin SDK developers get started by installing the necessary dependencies for the project. 
Please be advised that these scripts will install software to your computer and sometimes modify your environment, such as adding applications to the path in ~/.basrc. 
Administrator privileges may be required and users should ensure that they understand the impact of running the scripts before running them.

# Getting Started

To install all the dependencies for every part of this project, run `./install-dependencies.sh
--all`. Individual components can be installed by running each script independently. To install the 
dependencies for a particular part of the project, an option may be provided to 
`./install-dependencies.sh`. To view the available options run `./install-dependencies.sh --help`.

## install-bookdown.sh

This script installs R, pandoc, and bookdown, which is required for generating the Developer's
 Guide and QuickStart Guide from the RMarkdown source.

## install-boost.sh

This script installs boost, which is required to compile the RStudio Launcher Plugin SDK. The 
current version that will be installed is Boost 1.70.0.

## install-build-tools.sh

This script installs tools required for building the RStudio Launcher Plugin SDK and the 
accompanying RStudio Launcher Plugins, such as cmake, gcc, and g++.

## install-doxygen.sh

This script installs Doxygen, TexLive, Python and other necessary dependencies of Doxygen. It is
not needed unless you wish to regenerate the Doxygen documentation.

## install-rsandbox.sh

This script installs the `rsandbox` executable from the RStudio Server Pro session components. It is
required by the RStudio Launcher Plugin SDK to run child processes. Usage: 
`./install-rsandbox.sh <platform>`. Valid platforms are:
* `centos8`
* `debian9`
