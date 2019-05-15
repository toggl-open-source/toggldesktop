Table of Contents
=================

  * [Build instructions](#build-instructions)
    * [Build using Makefile](#build-using-makefile)
    * [Linux](#linux)
      * [Dependencies](#dependencies)
      * [Build the app](#build-the-app)
    * [Windows](#windows)
      * [Dependencies](#dependencies-1)
        * [Install dev tools](#install-dev-tools)
        * [Build OpenSSL](#build-openssl)
      * [Build the app](#build-the-app-1)
  * [Downloads](#downloads)
    * [macOS](#macos)
    * [Windows](#windows)
    * [Linux (64 bit only)](#linux-64-bit-only)
  * [Change log](#change-log)
  * [Documentation](#documentation)
  * [Contribute](#contribute)

# Build instructions

Please check OS specific requirements below.

_By default the app builds for testing server. To use the compiled app with live server see this guide [https://github.com/toggl/toggldesktop/wiki/Building-Toggl-Desktop-from-source-for-usage-with-live-servers](https://github.com/toggl/toggldesktop/wiki/Building-Toggl-Desktop-from-source-for-usage-with-live-servers)_

## Build using Makefile

First, build dependencies:
```
make deps
```
then the app itself:
```
make
```

To build, then run the app:
```
make run
```

Run unit tests with
```
make test
```
and UI tests with
```
make uitest
```

## Linux

### Dependencies

You'll need these Qt (at version 5.12 or higher) modules: QtWidgets (with private headers), QtWebEngine, QtWebEngineWidgets, QtNetwork, QtDBus, QtX11Extras

If Qt is not installed from your distribution's package manager, you will need to set the `CMAKE_PREFIX_PATH` environment variable to point to the `lib/cmake` folder in the Qt version you wish to use.

These dependencies are mandatory:
 * libXScrnSaver (`libxss-dev` in deb-based distros and `libXScrnSaver-devel` in rpm-based)

 You can install them all in debian with a command:
 ```
 sudo apt install libxss-dev build-essential libgl-dev libreadline-dev

 ```
 
These dependencies are optional and will be bundled if the `USE_BUNDLED_LIBRARIES` CMake argument is set or your system does NOT have their development packages installed:
 * POCO
 * Lua
 * jsoncpp
 * Qxt

These libraries will be bundled regardless of your system:
 * bugsnag-qt
 * qt-oauth-lib

### Build the app

*in the toggldesktop source tree root*
```
mkdir -p build && pushd build             # Create build directory
cmake ..                                  # Setup cmake configs
make -j8                                  # Build the app. The number defines the count of parallel jobs (number of your CPU cores is a good value for that)
./src/ui/linux/TogglDesktop/TogglDesktop  # Run the built app
```

## Windows

### Dependencies

Install Visual Studio 2013 to get Visual C++ 2013 Build Tools (v120). You can download it [here](https://visualstudio.microsoft.com/vs/older-downloads/).

Build the app using any Visual Studio 2013 or above. You can download free Visual Studio Community [here](https://visualstudio.microsoft.com/vs/community/).

If you want to run tests or build the app using make, then install [Cygwin](https://www.cygwin.com/).

### Build the app

Now, select *Release_VS* from the Solution Configurations combobox in the Visual Studio toolbar, and build the solution.

If you have Cygwin installed, you can also build the app from from Cygwin terminal, using make. (See macOS and Linux build instructions on above regarding make).

If you want to build OpenSSL from sources, refer to [this page](docs/win/build-openSSL.md).

# Downloads

## macOS

Toggl built and signed app for macOS is [available for download](https://www.toggl.com/api/v8/installer?platform=darwin&app=td&channel=stable). Officially macOS 10.11 and newer stable macOS versions are supported.

## Windows

Toggl built and signed app for Windows is [available for download](https://www.toggl.com/api/v8/installer?platform=windows&app=td&channel=stable). App has been tested on Windows 7, 8, 8.1 and 10.

Toggl Desktop Windows app has not been tested on Surface type touchscreen environments.

## Linux (64 bit only)

* [flatpak(recommended)](https://flathub.org/apps/details/com.toggl.TogglDesktop)
* [tarball](https://www.toggl.com/api/v8/installer?app=td&platform=linux&channel=stable)


# Change log

Change log can be viewed at [http://toggl.github.io/toggldesktop/](http://toggl.github.io/toggldesktop/)

# Documentation

The documentation of the code base is currently work in progress and can be found at [/docs/](https://github.com/toggl/toggldesktop/tree/master/docs/index.md).

# Contribute

Before sending us a pull request, please format the source code:

```
make fmt
```

Also, please check for any cpplint issues:

```
make lint
```

Check if unit tests continue to pass:

```
make test
```

