<h1 align="center">
  <a href="https://toggl.com"><img src="https://toggl.com/site/images/media-toolkit/logo_02-644bd26148b73c19d9c91e5baecd8e31.jpg" alt="Toggl" width="200"></a>
</h1>

<h4 align="center">Native desktop applications for the leading time tracking tool <a href="https://toggl.com" target="_blank">Toggl</a>.</h4>

<p align="center">
    <a href="https://github.com/toggl/toggldesktop/commits/master">
    <img src="https://img.shields.io/github/last-commit/toggl/toggldesktop.svg?style=flat&logo=github&logoColor=white"
         alt="GitHub last commit">
    <a href="https://github.com/toggl/toggldesktop/issues">
    <img src="https://img.shields.io/github/issues-raw/toggl/toggldesktop.svg?style=flat&logo=github&logoColor=white"
         alt="GitHub issues">
    <a href="https://github.com/toggl/toggldesktop/pulls">
    <img src="https://img.shields.io/github/issues-pr-raw/toggl/toggldesktop.svg?style=flat&logo=github&logoColor=white"
         alt="GitHub pull requests">
    <img src="https://img.shields.io/badge/licence-BSD--3-green"
         alt="Licence BSD-3">
</p>

<p align="center">
  <a href="#about">About</a> •
  <a href="#download">Download</a> •
  <a href="#build">Build</a> •
  <a href="#change-log">Change log</a> •
  <a href="#contribute">Contribute</a>
</p>

# About

  **Toggl Desktop** is a Toggl time tracking client with many helper functions that make tracking time more effortless and smooth. Features such as Idle detection, reminders to track and Pomodoro Timer make this app a great companion when productivity and efficiency is the goal.

<img src="https://user-images.githubusercontent.com/842229/63856838-3a869580-c9ab-11e9-9e36-7db23059ce29.png"
         alt="Toggl Desktop apps">

# Download

Toggl built and signed apps for all platforms

## Mac

<br>
<a href="https://toggl.github.io/toggldesktop/download/macos-stable/">64bit dmg</a>&nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;<a href='https://itunes.apple.com/ee/app/toggl-desktop/id957734279?mt=12'>
  Mac App Store</a>
<br/>
<br/>
<i>Officially macOS 10.11 and newer stable macOS versions are supported.</i>

## Windows

<br/>
<a href="https://toggl.github.io/toggldesktop/download/windows64-stable/">64bit installer</a>&nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;<a href="https://toggl.github.io/toggldesktop/download/windows-stable/">32bit installer</a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;<a href="https://chocolatey.org/packages/toggl">Chocolatey</a>&nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;<a href='//www.microsoft.com/store/apps/9nk3rf9nbjnp?cid=storebadge&ocid=badge'>Microsoft Store</a>
<br/>
<br/>
<i>App has been tested on Windows 7, 8, 8.1 and 10. Toggl Desktop Windows app has not been tested on Surface type touchscreen environments.</i>

## Linux

<br>
<a href="https://toggl.github.io/toggldesktop/download/linux_tar.gz-stable//">Tarball</a>&nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;<a href='https://flathub.org/apps/details/com.toggl.TogglDesktop'>Flathub</a>&nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;<a href='https://snapcraft.io/toggldesktop'>Snap Store</a>(still in alpha)
<br/>
<br/>
<i>Only 64bit is supported</i>

# Build

Please check OS specific requirements below.

_By default the app builds for testing server. To use the compiled app with live server see this guide [https://github.com/toggl/toggldesktop/wiki/Building-Toggl-Desktop-from-source-for-usage-with-live-servers](https://github.com/toggl/toggldesktop/wiki/Building-Toggl-Desktop-from-source-for-usage-with-live-servers)_

## Build using Makefile

First, build dependencies:
```bash
$ make deps
```
then the app itself:
```bash
$ make
```

To build, then run the app:
```bash
$ make run
```

Run unit tests with
```bash
$ make test
```
and UI tests with
```bash
$ make uitest
```
## macOS
### Requirements
- macOS 10.14+, Xcode 10.2+ and Swift 4+

### Build
```bash

# Build all dependencies
$ make deps

# Prepare cocoapod
$ make init_cocoapod
```
- Open workspace at `src/ui/osx/TogglDesktop.xcworkspace`
- Select TogglDesktop scheme and build.

## Linux

### Dependencies

You'll need these Qt (at version 5.12 or higher) modules: QtWidgets (with private headers), QtWebEngine, QtWebEngineWidgets, QtNetwork, QtDBus, QtX11Extras

If Qt is not installed from your distribution's package manager, you will need to set the `CMAKE_PREFIX_PATH` environment variable to point to the `lib/cmake` folder in the Qt version you wish to use.

These dependencies are mandatory:
 * libXScrnSaver (`libxss-dev` in deb-based distros and `libXScrnSaver-devel` in rpm-based)

 You can install them all in debian with a command:
```bash
 $ sudo apt install libxss-dev build-essential libgl-dev libreadline-dev

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
```bash
mkdir -p build && pushd build             # Create build directory
cmake ..                                  # Setup cmake configs
make -j8                                  # Build the app. The number defines the count of parallel jobs (number of your CPU cores is a good value for that)
./src/ui/linux/TogglDesktop/TogglDesktop  # Run the built app
```

## Windows

Install Visual Studio 2019 with `.NET desktop development`, `Desktop development with C++` and `Universal Windows Platform development` components checked during installation. You can download free Visual Studio Community [here](https://visualstudio.microsoft.com/vs/community/).

The solution is using OpenSSL binaries. To rebuild OpenSSL from sources refer to [this page](docs/win/build-openSSL.md).


# Change log

Change log can be viewed at [http://toggl.github.io/toggldesktop/](http://toggl.github.io/toggldesktop/)

# Contribute

Before sending us a pull request, please format the source code:

```bash
$ make fmt
```

Also, please check for any cpplint issues:

```bash
$ make lint
```

Check if unit tests continue to pass:

```bash
$ make test
```

