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
* Build dependencies

```
make deps
```

* Build kopsik itself:

```
make
```

An executable named *kopsik* should emerge as the result of the build.

