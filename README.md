# Cortex-M Scheduler

## Table of contents

1. [About the Project](#about-the-project)
1. [Project Status](#project-status)
1. [Getting Started](#getting-started)
1. [Dependencies](#dependencies)
1. [System Test](#system-test)
1. [Authors and Contributors](#authors-and-contributors)
1. [Contribution guidelines for this project](CONTRIBUTING.md)

## About the Project

This project is a priority-based preemptive scheduler for ARM Cortex-M3 devices. Hopefully we will be adding support for multiple architectures once the Cortex-M implementation is stable enough.

Currently focusing in the inexpensive STM32F103 Blue Pill as development platform (Cortex-M3).

## Project status

The project is in the very early stages. Currently there is a working priority-based scheduler implementation, but no priority inversion prevention methods have been implemented, and scheduling is definitely not optimal.

You are more than welcome if you want to contribute to the code. I will be adding contribution guidelines in the future.

## Getting Started

After cloning the repository and submodules just execute make to build all targets, including unit testing and the system test. It is possible to use the [ATE Builder](https://hub.docker.com/r/javiervarez/ate_builder) docker image to build and execute unit tests, since all dependencies are preinstalled.

Build the code running `make`.

Flash the board running `make flash`. For this step to work, you need to be using an st-link connected via SWD.

All targets are located at `build/targets`. It is possible to run the unittests by running `TestOS` in the targets directory.
In order to build the tests, the following environment variables need to be defined:

* `GOOGLETEST_LIBS_DIR`: Contains the output location of Google Test and Google Mock libraries.
* `GOOGLEMOCK_INCLUDE_DIR`: Contains the include directory of Google Mock.
* `GOOGLETEST_INCLUDE_DIR`: Contains the include directory of Google Test.

It would be interesting to get this up and running in [QEMU](https://xpack.github.io/qemu-arm/), but so far we had no time for this.

## Dependencies

* [ARM Embedded toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm). Build system enforces the use of version `9.2.1`.
* [Google Test and Google Mock](https://github.com/google/googletest). `v1.10`.
* [STM32CubeF1](https://github.com/STMicroelectronics/STM32CubeF1). This is automatically included as a submodule. Using version `v1.8.0`.
* [openocd](http://openocd.org/). Only needed for flashing/debugging. Using version `v0.10.0`.

## System test

A system test is run in the CI/CD environment for every commit. It uses a Saleae logic analyzer to capture 2 output signals that are controlled from 2 different tasks in the OS and should be toggling at a specific rate. If the toggling frequency tolerance is not met, the test fails.

## Authors and contributors

### Authors

* **[Javier Alvarez](https://gitlab.allthingsembedded.net/Javier_varez)** - *Initial work* - [AllThingsEmbedded](https://allthingsembedded.net/)

### Contributors

* **[Isaac Lacoba](https://gitlab.allthingsembedded.net/isaac.lacoba)**
* **[Javier Lancha](https://gitlab.allthingsembedded.net/jLantxa)**

## Acknowledgments

Special thanks to [Phillip Johnston](https://github.com/phillipjohnston) from [Embedded Artistry](https://github.com/embeddedartistry) for providing a great README.md template for this project.

**[Back to top](#table-of-contents)**
