Qt Solutions Component: Locked File

The QtLockedFile class extends QFile with advisory locking
functions.

This class extends the QFile class with inter-process file locking
capabilities. If an application requires that several processes
should access the same file, QtLockedFile can be used to easily
ensure that only one process at a time is writing to the file, and
that no process is writing to it while others are reading it.

Version history:

2.0: - This is a port to Qt 4 of the latest Qt 3 version (1.1).

2.1: - Fix non-blocking lock() on Windows.

2.2: - Windows: Possible deadlock removed.
     - Windows: Allow file names with paths.
     - Windows: Compilation fix for MinGW.

2.3: - Windows: Bugfix; non-blocking lock attempt could cause
     locking failures later.

2.4: - Misc. minor fixes.
     - Windows: Reimplemented. Fixed issue where crashed processes
     could hold on to a lock.
     - LGPL release.

