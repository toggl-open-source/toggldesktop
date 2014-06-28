Qt Solutions Component: Browser Plugin

The QtBrowserPlugin solution is useful for implementing plugins
for web browser.



Notes:

This module can not function if built as a DLL, it must be
statically linked with the plugin. Hence, DLL building is not
supported, and the -library option to configure will be ignored.
For the same reason, the source code that needs to be statically
linked is licensed under a BSD style license (ref. the comment
headers of the files in the src directory).

Version 2.4 is known to not work, even experimentally, on Mac OS
X.

Version history:

2.1: - Documentation updates.
     - Support of shortcut keys in the plugin (Windows only)
     - Implement scripting support.
     - Add experimental support for Mac (Safari) and Linux (Firefox).

2.2: - Fix crash when calling slots with multiple arguments of
     different types from script.
     - Fix compilation and potential stability problem on 64 bit
     architectures.

2.3: - Support default and QVariant-type parameters in script
     binding.
     - Be more selective in setting plugin widget's autofillbackground
     to true - Avoids conflict with OpenGL.
     - Fixed bug where QtBrowserPlugin could not change properties on
     sub-objects.
     - Fixed default mimeTypes() implementations. Makes it work with
     Opera 9.50 on Linux.
     - For Qt 4.4, support for using a namespaced Qt build to avoid
     symbol clash with other instances of Qt in the browser.

2.4: - Don't trigger first QAction without shortcut if
     non-alphanumerical key is pressed.
     - Documentation updates.
     - Misc. minor fixes.
     - LGPL/BSD release.

