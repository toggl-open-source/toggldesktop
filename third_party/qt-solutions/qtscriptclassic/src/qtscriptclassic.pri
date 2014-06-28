INCLUDEPATH += $$PWD/../include $$PWD
DEPENDPATH += $$PWD
TEMPLATE += fakelib
mac:LIBS += -F$$PWD/../lib -framework QtScriptClassic
else:LIBS += -L$$PWD/../lib -l$$qtLibraryTarget(QtScriptClassic)
TEMPLATE -= fakelib
unix:QMAKE_RPATHDIR += $$PWD/../lib
