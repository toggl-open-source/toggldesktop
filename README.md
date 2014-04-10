[![Build Status](https://drone.io/github.com/toggl/toggldesktop/status.png)](https://drone.io/github.com/toggl/toggldesktop/latest)

[![Build status](https://ci.appveyor.com/api/projects/status/8uic9ed9xyspt87f)](https://ci.appveyor.com/project/tanel/toggl-toggldesktop)

[![Build Status](https://travis-ci.org/toggl/toggldesktop.png)](https://travis-ci.org/toggl/toggldesktop)


Project dependencies
--------------------
First, build dependencies:
```
make deps
```

OS X
---
```
make osx
```

Toggl built and signed app is [available for download](https://www.toggl.com/api/v8/installer?platform=darwin&app=td&channel=stable). Currently OSX 10.8 is the minimum version supported.

Windows
-------
First, [build OpenSSL for Visual Studio](http://developer.covenanteyes.com/building-openssl-for-visual-studio/)

We're building the Windows app using [Visual Studio Express 2013 for Windows Desktop](http://www.microsoft.com/en-us/download/details.aspx?id=40787)

Linux
-----
We haven't started work on Linux UI yet. 


Cross platform command line app
-------------------------------
```
make
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
open coverage/index.html
```

