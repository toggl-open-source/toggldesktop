
# Toggl Desktop Documentation

The documentation for the Toggl Desktop code base is currently work in progress.

## Introduction

Toggl Desktop is a crossplatform native desktop application that is an extension of the madly simple time tracking tool Toggl. The app is developed in three big C-s: C++, Objective-C and C#.

Toggl Desktop is composed by 4 bigger parts:
  - Library
  - Windows User Interface
  - OS X User Interface
  - Linux User Interface

### Library
Library is developed in C++ and this is the heart and soul of the whole application. Library is the one that connects to the servers, saves data into local database and syncs data between local database and the online one. Most of the actions done in Toggl Desktop are either triggered from the library or will trigger something in the library.

_More detailed description is available at [Library Documentation](lib/index.md)_

### Windows UI
Windows app is built in C# and the powerful WPF (Windows Presentation Foundation). Windows UI communicates with the library through a special wrapper defined with `TogglApi.cs`. This file is generated automatically by running command `make csapi`.

Windows app is packaged in [NSIS (Nullsoft Scriptable Install System)](http://nsis.sourceforge.net/Main_Page) based installer. Windows app is meant to be used without administrative rights. The app installs itself into the user directory `C:\Users\<username>\AppData\Local\TogglDesktop`

Windows application works with 64 bit architecture on Windows 7 and newer

_More detailed description is available at [Windows UI Documentation](win/index.md)_

### OS X UI
Mac app is built in Objective-C with Cocoa. The app communicates with library by connections of functions that are described in `AppDelegate.m` in the `init` method. What it basically does is connects methods described in AppDelegate with methods in library.

There are two release versions of the OS X app. `.dmg` and AppStore. The AppStore version has been lacking behind as there have been several issues with deployment and review process.

OS X application works with 64 bit architecture on OS X 10.8 Mountain Lion and newer

_More detailed description is available at [OS X UI Documentation](mac/index.md)_

### Linux UI
Linux app is built in C++ with [Qt 5.5](http://www.qt.io/). Linux app communicates with library throught `toggl.cpp` file located in `src/ui/linux/TogglDesktop`. This file assigns the methods that are use in the Qt based UI code to the library functions.

The Linux version has two different packages `tar.gz` for all Linux distributions and `.deb` for debian based Linux machines.

Linux application only works with 64 bit architecture

_More detailed description is available at [Linux UI Documentation](linux/index.md)_

## Various

- [Folder structure](various/folders.md)
- [Format and Lint](various/fmt.md)
- [Make commands](various/make.md)
- [Tests](various/tests.md)
