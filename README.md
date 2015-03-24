
[![Build Status](https://travis-ci.org/toggl/toggldesktop.png)](https://travis-ci.org/toggl/toggldesktop)

Table of Contents
=================

  * [Build instructions](#build-instructions)
    * [Build using Makefile](#build-using-makefile)
    * [Linux](#linux)
    * [Debian](#debian)
    * [Ubuntu](#ubuntu)
    * [Fedora](#fedora)
    * [Windows](#windows)
      * [Install dev tools](#install-dev-tools)
      * [Build OpenSSL](#build-openssl)
      * [Install NuGet packages](#install-nuget-packages)
      * [Build the app](#build-the-app)
  * [Downloads](#downloads)
    * [OSX](#osx)
    * [Windows](#windows-1)
    * [Linux (64 bit only)](#linux-64-bit-only)
  * [Contribute](#contribute)


# Build instructions

Please check OS specific requirements below.

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

You'll need QT to be at least version 5.2

If qmake is not on your PATH, set this first, before running make:

```
export QMAKE=/usr/bin/qmake-qt5
```

You also need libreadline-dev to build.

Below are some distribution specific package requirements:

## Debian

* qt5-default

## Ubuntu

* xorg-dev
* qtcreator
* libxss-dev
* libqt5webkit5-dev

## Fedora

* qt5-qtwebkit-devel
* libXScrnSaver-devel
* qt5-qtsvg

## Windows

### Install dev tools

We're building the Windows app using Visual Studio Community 2013 - it's a [free download](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx)

You'll need to [install ActivePerl](http://www.activestate.com/activeperl/downloads) to build OpenSSL from source.

If you plan to run tests then you might want to install [Cygwin](https://www.cygwin.com/).

### Build OpenSSL

To build OpenSSL, from Visual Studio Tools, open up a Developer Command Prompt.

cd to the project folder, then

```
cd third_party\openssl
perl Configure VC-WIN32
ms\do_nasm
nmake -f ms\ntdll.mak clean
nmake -f ms\ntdll.mak 
```

Instead of do_nasm (use NASM) you can also use do_ms (no asm at all), or do_masm (use MASM). 
NASM can be downloaded here: http://www.nasm.us/pub/nasm/releasebuilds/?C=M;O=D

### Install NuGet packages

Open the solution in Visual Studio. Next, you'll need to install the net-bugsnag package: from the Tools menu select NuGet Package Manager, then Package Manager Console. Into the console, type:

```
Install-Package Bugsnag.Library
```

From the same console, install Oauth2 related packages:

```
Install-Package Google.Apis.Auth;
Install-Package Google.Apis.Oauth2.v2;
```

### Build the app

Now, select *Release* from the Solution Configurations combobox in the Visual Studio toolbar, and build the solution.

If you have Cygwin installed, you can also build the app from from Cygwin terminal, using make. (See OSX and Linux build instructions on above regarding make).

# Downloads

## OSX

Toggl built and signed app for OSX is [available for download](https://www.toggl.com/api/v8/installer?platform=darwin&app=td&channel=stable). You need at least OSX 10.8.

## Windows

Toggl built and signed app for Windows is [available for download](https://www.toggl.com/api/v8/installer?platform=windows&app=td&channel=stable). App has been tested on Windows 7, 8 and 8.1

## Linux (64 bit only)

* [tarball](https://www.toggl.com/api/v8/installer?app=td&platform=linux&channel=stable)
* [deb](https://www.toggl.com/api/v8/installer?app=td&platform=deb64&channel=stable)


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

