
[![Build Status](https://travis-ci.org/toggl/toggldesktop.png)](https://travis-ci.org/toggl/toggldesktop)


OS X
---
First, build dependencies:
```
make deps
```
then the app itself:
```
make osx
```

To build and run the app:
```
make run
```

Toggl built and signed app is [available for download](https://www.toggl.com/api/v8/installer?platform=darwin&app=td&channel=stable). Currently OSX 10.8 is the minimum version supported.

Windows
-------
We're building the Windows app using [Visual Studio Express 2013 for Windows Desktop](http://www.microsoft.com/en-us/download/details.aspx?id=40787) 

First, [build OpenSSL for Visual Studio](http://developer.covenanteyes.com/building-openssl-for-visual-studio/). From Visual Studio Tools, open up a Developer Command Prompt. cd to the project folder, then

```
cd third_party\openssl
perl Configure VC-WIN32
ms\do_ms
nmake -f ms\ntdll.mak 
```

Then open the solution in Visual Studio. Next, you'll need to install the net-bugsnag package: from the Tools menu select NuGet Package Manager, then Package Manager Console. Into the console, type:

```
Install-Package Bugsnag.Library
```

No Toggl build and signed app is available yet.

Linux
-----
We haven't started work on Linux UI yet. 


Tests
-----
```
make test
```

