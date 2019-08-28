<style>
  a.link {
    border: 1px solid #000;
    display: block;
    width: 180px;
    height: 60px;
    border-radius: 9px;
    line-height: 60px;
    vertical-align: middle;
  }
  table.linktable {
    border:none;
    display: table;
  }
  table.linktable tr {
    border: none;
    background-color: #fff!important;
  }
  table.linktable td {
    border:none;
  }
  table.linktable td .win-link {
    margin-top: 10px;
  }
  table.linktable td .mac-link {
    margin-top: 6px;
  }
  table.linktable p {
    width: 180px;
    position: relative;
    bottom: 0;
  }
  .disabled:before {
    content: 'Coming soon';
    background-color: #9a9797c7;
    color: white;
    position: absolute;
    height: 60px;
    text-align: center;
    vertical-align: bottom;
    line-height: 60px;
    width: 180px;
  }
</style>

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
  <a href="#features">About</a> •
  <a href="#download">Download</a> •
  <a href="#build">Build</a> •
  <a href="#change-log">Change log</a> •
  <a href="#contribute">Contribute</a>
</p>

## About

<table>
<tr>
<td>

  **Toggl Desktop** is a Toggl time tracking client with many helper functionas that make tracking time more effortless and smooth. Features such as Idle detection, reminders to track and Pomodoro Timer make this app a great companion when productivity and efficiancy is the goal

</td>
</tr>
</table>

Screenshot

## Download

Toggl built and signed apps for all platforms

<table class="linktable">
  <tr align="center">
    <td valign="top">
      <p>Windows</p>
      <br/>
      <a class="link" href="https://toggl.github.io/toggldesktop/download/windows-stable/">32bit</a>
      <br/>
      <a class="link" href="https://toggl.github.io/toggldesktop/download/windows64-stable/">64bit</a>
      <br/>
      <a class="link" href='//www.microsoft.com/store/apps/9nk3rf9nbjnp?cid=storebadge&ocid=badge'><img class="win-link" src='https://user-images.githubusercontent.com/842229/63852913-13779600-c9a2-11e9-87bd-9898ece07148.png' alt='Download Toggl Desktop on Microsoft Store'/></a>
    </td>
    <td valign="top">
      <p>Mac</p>
      <br>
      <a class="link" href="https://toggl.github.io/toggldesktop/download/macos-stable/">64bit</a>
      <br>
      <a class="link" href='https://itunes.apple.com/ee/app/toggl-desktop/id957734279?mt=12'><img class="mac-link" alt='Download Toggl Desktop on the App Store' src='https://user-images.githubusercontent.com/842229/63853927-9863af00-c9a4-11e9-8c5d-c00f0c7b7536.png'/></a>
      <br/>
      <br/>
    </td>
    <td valign="top">
      <p>Linux</p>
      <br>
      <a class="link" href="https://toggl.github.io/toggldesktop/download/linux_tar.gz-stable//">Tarball</a>
      <br>
      <a href='https://flathub.org/apps/details/com.toggl.TogglDesktop'><img width='180' alt='Download Toggl Desktop on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.svg'/></a>
      <br>
      <br>
      <!--a href='#' class="disabled"><img width='180' alt='Download Toggl Desktop on Snapcraft' src='https://raw.githubusercontent.com/snapcore/snap-store-badges/master/EN/%5BEN%5D-snap-store-white%402x.png'/></a-->
    </td>
  </tr>
  <tr align="center">
    <td valign="top">
      <p style="font-size:12px">
        App has been tested on Windows 7, 8, 8.1 and 10
        </br>
        </br>
        <i>Toggl Desktop Windows app has not been tested on Surface type touchscreen environments.</i>
      </p>
    </td>
    <td valign="top">
      <p style="font-size:12px">
        Officially macOS 10.11 and newer stable macOS versions are supported.
      </p>
    </td>
    <td valign="top">
      <p style="font-size:12px">
        Only 64bit is supported
      </p>
    </td>
  </tr>
</table>
</p>

## Build

Please check OS specific requirements below.

_By default the app builds for testing server. To use the compiled app with live server see this guide [https://github.com/toggl/toggldesktop/wiki/Building-Toggl-Desktop-from-source-for-usage-with-live-servers](https://github.com/toggl/toggldesktop/wiki/Building-Toggl-Desktop-from-source-for-usage-with-live-servers)_

### Build using Makefile

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

### Linux

#### Dependencies

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

#### Build the app

*in the toggldesktop source tree root*
```
mkdir -p build && pushd build             # Create build directory
cmake ..                                  # Setup cmake configs
make -j8                                  # Build the app. The number defines the count of parallel jobs (number of your CPU cores is a good value for that)
./src/ui/linux/TogglDesktop/TogglDesktop  # Run the built app
```

### Windows

Install Visual Studio 2019 with `.NET desktop development`, `Desktop development with C++` and `Universal Windows Platform development` components checked during installation. You can download free Visual Studio Community [here](https://visualstudio.microsoft.com/vs/community/).

The solution is using OpenSSL binaries. To rebuild OpenSSL from sources refer to [this page](docs/win/build-openSSL.md).


## Change log

Change log can be viewed at [http://toggl.github.io/toggldesktop/](http://toggl.github.io/toggldesktop/)

## Contribute

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

