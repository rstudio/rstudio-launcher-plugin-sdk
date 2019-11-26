# Purpose

The scripts in this folder help new RStudio Launcher Plugin SDK developers get started by installing the necessary dependencies for the project. Please be advised that these scripts will install software to your computer and sometimes modify your environment, such as adding applications to the path in ~/.basrc. Administrator privileges may be required and users should take care before running the scripts.

# Getting Started

To install all the dependencies for every part of this project, run `./install-dependencies.sh --all`. Individual components can be installed by running each script independently. To install the dependencies for a particular part of the project, an option may be provided to `./install-dependencies.sh`. To view the available options run `./install-dependencies.sh --help`.

## install-boost.sh

This script installs boost, which is required to compile the RStudio Launcher Plugin SDK. The current version that will be installed is Boost 1.70.0.

## install-build-tools.sh

This script installs tools required for building the RStudio Launcher Plugin SDK and the accompanying RStudio Launcher Plugins, such as cmake, gcc, and g++.

## install-singularity.sh

This script installs [Singularity](https://sylabs.io/singularity/), [Go](https://golang.org/), and all other dependencies of Singularity. The ~/.bashrc file will be updated so that both Singularity and Go will be on the path each time you login, and to enable tab-based auto-completion for Singularity commands. Singularity is required to use the Singularity Launcher Plugin, which is provided with this SDK as a sample of a complete and working Launcher Plugin.