// Copyright 2014 Tanel Lebedev.

#ifndef BUGSNAG_QT_GLOBAL_H_
#define BUGSNAG_QT_GLOBAL_H_

#include <QtCore/qglobal.h>

#if defined(BUGSNAGQT_LIBRARY)
#  define BUGSNAGQTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BUGSNAGQTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif  // BUGSNAG_QT_GLOBAL_H_
