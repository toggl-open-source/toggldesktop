kopsik
======

Experimental Toggl API client in good ol' C++


Features
========
1. Basic Toggl API models: user, workspace, project, task, time entry, tag
2. Authenticate user via Toggl API
3. User can track time.
4. Sync pushes/pulls all changes once
5. Local storage for offline usage. Maybe sqlite.
6. Receive Toggl data updates using WebSocket
7. Command line client for testing and as an example on how to use.


Technicalities
==============
* [POCO](http://pocoproject.org/) for networking, storage etc


Install
=======
* Build POCO

```
cd third_party/poco-1.4.6p1-all
./configure --omit=Data/ODBC,Data/MySQL
make
```

* Build openSSL

