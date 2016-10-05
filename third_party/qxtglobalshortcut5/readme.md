### Qxt qxtglobalshortcut qt5
---

[Qxt ](http://dev.libqxt.org/libqxt/wiki/Home) qxtglobalshortcut library qt5 compatibility
Tested with qt5.4 on OSX 10.9.2, linux, windows (mingw and msvc)

usage:

```
git clone https://github.com/ddqd/qxtglobalshortcut5.git
```

add in ur .pro file:
```
include(qxtglobalshortcut5/qxt.pri)
```

example code:
```cpp
#include <QDebug>
#include "qxtglobalshortcut.h"
...

QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
shortcut->setShortcut(QKeySequence("Shift+1"));
connect(shortcut, &QxtGlobalShortcut::activated,
    [=]() {qDebug() << "shortcut activated";});
```
