[![Build Status](https://travis-ci.org/toggl/toggldesktop.png)](https://travis-ci.org/toggl/toggldesktop)

OS X
----
```
make osx
```
Toggl built and signed app is [available for download](https://www.toggl.com/api/v8/installer?platform=darwin&app=td&channel=stable).

XCode Version 5.0.2 is required to hack on the project.

Currently OSX 10.8 is the minimum version supported.

Windows
-------
* [build OpenSSL for Visual Studio](http://developer.covenanteyes.com/building-openssl-for-visual-studio/)
* [build POCO, using Visual Studio](https://github.com/pocoproject/poco/blob/develop/README)

We're building the Windows app using [Visual Studio Express 2013 for Windows Desktop](http://www.microsoft.com/en-us/download/details.aspx?id=40787)

Linux
-----
FIXME


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

