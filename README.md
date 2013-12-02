kopsik
======

Experimental natitve Toggl API client, using [POCO](http://pocoproject.org/)

[![Build Status](https://travis-ci.org/tanel/kopsik.png)](https://travis-ci.org/tanel/kopsik)

Command line client
-------------------
The command line app expectes to find *TOGGL_API_TOKEN* in your environment. You could export it in your ~/.bash_profile:

Recognized commands are: sync, start, stop, status, pushable, list, continue, listen

```
export TOGGL_API_TOKEN=<your Toggl API token here>
```

To build command line app, run:
```
make
```
An executable named *toggl* should emerge as the result of the build. Run it, to see available commands:
```
./toggl
```

OS X app
--------
```
make osx
```

Tests
-----
```
make test
```

Code coverage
-------------
```
make coverage
```
