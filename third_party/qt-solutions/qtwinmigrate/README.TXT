Qt Solutions Component: Qt/MFC Migration Framework

The Qt/MFC Migration Framework tool assists in the migration of
existing Win32 or MFC applications to the Qt toolkit.

The project provides a set of classes that allow using Qt and
MFC/Win32 windows in the same application. User interface elements
developed with Qt can be embedded into an existing MFC or Win32
based user interface, and existing custom controls developed with
MFC or Win32 can be integrated into Qt widgets.

Notes:

When building this component as a library ("configure.bat
-library"), the default configuration includes MFC support. That
is, the MFC SDK must be present, otherwise compilation will fail.
To avoid this dependency (e.g. for usage with Visual Studio
Express editions, which do not include MFC), edit the
src/qtwinmigrate.pri file as explained in its comments before
building this component.

Version history:

1.1: - Block the Win32 window when a modal Qt dialog opens.

1.2: - Integrate Qt and Win32 focus chains for QWinWidget.

2.0: - Version 1.2 ported to Qt 4.

2.1: - Fix compilation issue in "winwidget" example.
     - Don't close when the last Qt widget closes.
     - Improve idle-processing.
     - Make MFC accelerators work.

2.2: - Compilation and documentation fixes.

2.3: - Fix possible endless recursion when using QWinWidget in an
     MFC dialog.
     - Make sure that widgets paint focus indicators when tabbed into
     coming from Win32 windows.

2.4: - Make QtWinWidget center properly on parent widget when
     showCentered().
     - Do not activate Win32 window when a nested Qt dialog is opened.

2.5: - Make QWinHost a Q_OBJECT to allow safe casting via
     qobject_cast.
     - Assert if window handles passed in are invalid.
     - Clarify documentation.
     - New API QWinWidget::center that does not show the widget.
     - Fix type of idle-counter to make OnIdle handling work as in MFC.

2.6: - Provide API to block deferred deletes from being processed.
     - Compile without Qt3Support.
     - Support modal QWinWidget-children that are not QDialogs.

2.7: - Fix potential infinite loop when setting focus.

2.8: - Misc. minor fixes.
     - LGPL release.

