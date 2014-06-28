Qt Solutions Component: QtScript Classic

This component provides the original (pre-Qt 4.6) implementation
of the QtScript module.



Notes:

QtScriptClassic is always built as a shared/dynamic library. The
library file is named QtScriptClassic, so as to differentiate it
from the QtScript module's, which is called QtScript.

To use QtScriptClassic in your project, include the
src/qtscriptclassic.pri file in your qmake project file, or make
the corresponding changes to your build setup. The project file
must not contain 'Qt += script', since that may cause the wrong
header files to be used and linking symbol clashes with the
QtScript module.

As per Qt 4.6, the API of QtScriptClassic is for most practical
purposes fully source compatible with the QtScript module.

Qt can be built without the QtScript module by giving the
-no-script option to configure.

Version history:

1.0: - Initial version, corresponding to Qt 4.6.0

