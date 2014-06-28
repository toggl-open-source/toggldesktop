/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QStringList>
#include <QPointer>
#include <QWidget>
#include <QEvent>
#include <QDebug>
#include <QRect>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QKeyEvent>

#include <Carbon/Carbon.h>

#include "qtnpapi.h"

#include "qtbrowserplugin.h"
#include "qtbrowserplugin_p.h"

static bool ownsqapp = false;
extern void qt_mac_set_native_menubar(bool b);
const UInt32 kWidgetCreatorQt = 'cute';
enum {
    kWidgetPropertyQWidget = 'QWId' //QWidget *
};

class
QMacBrowserRoot : public QWidget
{
    Q_OBJECT
public:
    QMacBrowserRoot(HIViewRef root) : QWidget()
    {
        // make sure we're not registered with Qt before create
        WindowRef window = HIViewGetWindow(root);
        QWidget *oldwindow=0;
        OSErr err;
        err = GetWindowProperty(window,
                                kWidgetCreatorQt,
                                kWidgetPropertyQWidget,
                                sizeof(oldwindow),
                                0,
                                &oldwindow);
        if (err == noErr)
            RemoveWindowProperty(window, kWidgetCreatorQt, kWidgetPropertyQWidget);

        create((WId)root);

        // re-register the root window with Qt
        err = SetWindowProperty(window,
                                kWidgetCreatorQt,
                                kWidgetPropertyQWidget,
                                sizeof(oldwindow),
                                &oldwindow);
        if (err != noErr) {
            qWarning("Error, couldn't register Window with Qt: (%s:%d:%d)", __FILE__, __LINE__, err);
        }

        QPalette pal = palette();
        pal.setColor(QPalette::Window,Qt::transparent);
        setPalette(pal);

        setAttribute(Qt::WA_WState_Polished);
    }

    ~QMacBrowserRoot() { }
};
#include "qtbrowserplugin_mac.moc"

struct key_sym
{
    int mac_code;
    int qt_code;
    const char *desc;
};

static key_sym modifier_syms[] = {
{ shiftKey, Qt::ShiftModifier, "Qt::ShiftModifier" },
{ controlKey, Qt::MetaModifier, "Qt::MetaModifier" },
{ rightControlKey, Qt::MetaModifier, "Qt::MetaModifier" },
{ optionKey, Qt::AltModifier, "Qt::AltModifier" },
{ rightOptionKey, Qt::AltModifier, "Qt::AltModifier" },
{ cmdKey, Qt::ControlModifier, "Qt::ControlModifier" },
{   0, 0, NULL }
};
static Qt::KeyboardModifiers get_modifiers(int key)
{
    Qt::KeyboardModifiers ret = 0;
    for(int i = 0; modifier_syms[i].desc; i++) {
        if(key & modifier_syms[i].mac_code) {
            ret |= Qt::KeyboardModifier(modifier_syms[i].qt_code);
        }
    }
    return ret;
}

static key_sym key_syms[] = {
{ kHomeCharCode, Qt::Key_Home, "Qt::Home" },
{ kEnterCharCode, Qt::Key_Enter, "Qt::Key_Enter" },
{ kEndCharCode, Qt::Key_End, "Qt::Key_End" },
{ kBackspaceCharCode, Qt::Key_Backspace, "Qt::Backspace" },
{ kTabCharCode, Qt::Key_Tab, "Qt::Tab" },
{ kPageUpCharCode, Qt::Key_PageUp, "Qt::PageUp" },
{ kPageDownCharCode, Qt::Key_PageDown, "Qt::PageDown" },
{ kReturnCharCode, Qt::Key_Return, "Qt::Key_Return" },
//function keys?
{ kEscapeCharCode, Qt::Key_Escape, "Qt::Key_Escape" },
{ kLeftArrowCharCode, Qt::Key_Left, "Qt::Key_Left" },
{ kRightArrowCharCode, Qt::Key_Right, "Qt::Key_Right" },
{ kUpArrowCharCode, Qt::Key_Up, "Qt::Key_Up" },
{ kDownArrowCharCode, Qt::Key_Down, "Qt::Key_Down" },
{ kDeleteCharCode, Qt::Key_Delete, "Qt::Key_Delete" }
};
static int get_key(int key)
{
    for(int i = 0; key_syms[i].desc; i++) {
        if(key_syms[i].mac_code == key) {
            return key_syms[i].qt_code;
        }
    }
    return key;
}

struct qt_last_mouse_down_struct {
    unsigned int when;
    int x, y;
} qt_last_mouse_down = { 0, 0, 0 };

//nasty, copied code -
static void qt_dispatchEnterLeave(QWidget* enter, QWidget* leave) {
#if 0
    if(leave) {
        QEvent e(QEvent::Leave);
        QApplication::sendEvent(leave, & e);
    }
    if(enter) {
        QEvent e(QEvent::Enter);
        QApplication::sendEvent(enter, & e);
    }
    return;
#endif

    QWidget* w ;
    if(!enter && !leave)
        return;
    QWidgetList leaveList;
    QWidgetList enterList;

    bool sameWindow = leave && enter && leave->window() == enter->window();
    if(leave && !sameWindow) {
        w = leave;
        do {
            leaveList.append(w);
        } while(!w->isWindow() && (w = w->parentWidget()));
    }
    if(enter && !sameWindow) {
        w = enter;
        do {
            enterList.prepend(w);
        } while(!w->isWindow() && (w = w->parentWidget()));
    }
    if(sameWindow) {
        int enterDepth = 0;
        int leaveDepth = 0;
        w = enter;
        while(!w->isWindow() && (w = w->parentWidget()))
            enterDepth++;
        w = leave;
        while(!w->isWindow() && (w = w->parentWidget()))
            leaveDepth++;
        QWidget* wenter = enter;
        QWidget* wleave = leave;
        while(enterDepth > leaveDepth) {
            wenter = wenter->parentWidget();
            enterDepth--;
        }
        while(leaveDepth > enterDepth) {
            wleave = wleave->parentWidget();
            leaveDepth--;
        }
        while(!wenter->isWindow() && wenter != wleave) {
            wenter = wenter->parentWidget();
            wleave = wleave->parentWidget();
        }

        w = leave;
        while(w != wleave) {
            leaveList.append(w);
            w = w->parentWidget();
        }
        w = enter;
        while(w != wenter) {
            enterList.prepend(w);
            w = w->parentWidget();
        }
    }

    QEvent leaveEvent(QEvent::Leave);
    for (int i = 0; i < leaveList.size(); ++i) {
        w = leaveList.at(i);
        QApplication::sendEvent(w, &leaveEvent);
#if 0
        if(w->testAttribute(Qt::WA_Hover)) {
            Q_ASSERT(instance());
            QHoverEvent he(QEvent::HoverLeave, QPoint(-1, -1),
                           w->mapFromGlobal(QApplicationPrivate::instance()->hoverGlobalPos));
            QApplication::sendEvent(w, &he);
        }
#endif
    }
    QPoint posEnter = QCursor::pos();
    QEvent enterEvent(QEvent::Enter);
    for (int i = 0; i < enterList.size(); ++i) {
        w = enterList.at(i);
        QApplication::sendEvent(w, &enterEvent);
        if(w->testAttribute(Qt::WA_Hover)) {
            QHoverEvent he(QEvent::HoverEnter, w->mapFromGlobal(posEnter), QPoint(-1, -1));
            QApplication::sendEvent(w, &he);
        }
    }
}


extern "C" bool qtns_event(QtNPInstance *This, NPEvent *event)
{
    static QPointer<QWidget> lastWidget;
    static QPointer<QWidget> qt_button_down;
    static Point lastPosition = { 0, 0 };
    if(event->what == nullEvent || event->what == adjustCursorEvent) {
        if(event->what == nullEvent) {
            qApp->processEvents();
            QApplication::sendPostedEvents();
        }

        //watch for mouse moves
       Point currentPosition;
        GetMouse(&currentPosition);
        LocalToGlobal(&currentPosition);
        if(currentPosition.h != lastPosition.h || currentPosition.v != lastPosition.v) {
            lastPosition = currentPosition;

            WindowPtr wp;
            FindWindow(currentPosition, &wp);
            QWidget *widget = 0;
            if(wp == GetWindowFromPort((CGrafPtr)This->window->port))
                widget = This->rootWidget->childAt(This->rootWidget->mapFromGlobal(QPoint(event->where.h, event->where.v)));
            else
                widget = QApplication::widgetAt(event->where.h, event->where.v);
            if(widget != lastWidget) {
                qt_dispatchEnterLeave(widget, lastWidget);
                lastWidget = widget;
            }
            if(widget) {
                QPoint p(currentPosition.h, currentPosition.v);
                QPoint plocal(widget->mapFromGlobal(p));
                QMouseEvent qme(QEvent::MouseMove, plocal, p, Button() ? Qt::LeftButton : Qt::NoButton,
                                0, get_modifiers(GetCurrentKeyModifiers()));
                QApplication::sendEvent(widget, &qme);
            }
        }
        return true;
    } else if(QWidget *widget = qobject_cast<QWidget*>(This->qt.object)) {
        if(event->what == updateEvt) {
            widget->repaint();
            return true;
        } else if(event->what == keyUp || event->what == keyDown) {
            QWidget *widget = 0;
            if(QWidget::keyboardGrabber())
                widget = QWidget::keyboardGrabber();
            else if(QApplication::focusWidget())
                widget = QApplication::focusWidget();
            else //last ditch effort
                widget = QApplication::widgetAt(event->where.h, event->where.v);

            if(widget) {
#if 0
                if(app_do_modal && !qt_try_modal(widget, er))
                    return 1;
#endif

                int mychar=get_key(event->message & charCodeMask);
                QEvent::Type etype = event->what == keyUp ? QEvent::KeyRelease : QEvent::KeyPress;
                QKeyEvent ke(etype, mychar, get_modifiers(event->modifiers), QString(QChar(mychar)));
                QApplication::sendEvent(widget,&ke);
                return true;
            }
        } else if(event->what == mouseDown  || event->what == mouseUp) {
            QEvent::Type etype = QEvent::None;
            Qt::KeyboardModifiers keys = get_modifiers(event->modifiers);
            Qt::MouseButton button = Qt::LeftButton;

            if(event->what == mouseDown) {
                if (lastWidget)
                    qt_button_down = lastWidget;
                //check if this is the second click, there must be a way to make the
                //mac do this for us, FIXME!!
                if(qt_last_mouse_down.when &&
                   (event->when - qt_last_mouse_down.when <= (uint)QApplication::doubleClickInterval())) {
                    int x = event->where.h, y = event->where.v;
                    if(x >= (qt_last_mouse_down.x-2) && x <= (qt_last_mouse_down.x+4) &&
                       y >= (qt_last_mouse_down.y-2) && y <= (qt_last_mouse_down.y+4)) {
                        etype = QEvent::MouseButtonDblClick;
                        qt_last_mouse_down.when = 0;
                    }
                }

                if(etype == QEvent::None) { //guess it's just a press
                    etype = QEvent::MouseButtonPress;
                    qt_last_mouse_down.when = event->when;
                    qt_last_mouse_down.x = event->where.h;
                    qt_last_mouse_down.y = event->where.v;
                }
            } else {
                etype = QEvent::MouseButtonRelease;
            }

            WindowPtr wp;
            FindWindow(event->where, &wp);

            //handle popup's first
            QWidget *popupwidget = NULL;
            if(QApplication::activePopupWidget()) {
                if(wp) {
                    QWidget *clt=QWidget::find((WId)wp);
                    if(clt && clt->windowType() == Qt::Popup)
                        popupwidget = clt;
                }
                if(!popupwidget)
                    popupwidget = QApplication::activePopupWidget();
                if(QWidget *child = popupwidget->childAt(popupwidget->mapFromGlobal(QPoint(event->where.h, event->where.v))))
                    popupwidget = child;

                QPoint p(event->where.h, event->where.v);
                QPoint plocal(popupwidget->mapFromGlobal(p));
                QMouseEvent qme(etype, plocal, p, button, 0, keys);
                QApplication::sendEvent(popupwidget, &qme);
            }

            {
                QWidget *widget = 0; //figure out which widget to send it to
                if(event->what == mouseUp && qt_button_down)
                    widget = qt_button_down;
                else if(QWidget::mouseGrabber())
                    widget = QWidget::mouseGrabber();
                else if(wp == GetWindowFromPort((CGrafPtr)This->window->port))
                    widget = This->rootWidget->childAt(This->rootWidget->mapFromGlobal(QPoint(event->where.h, event->where.v)));
                else
                    widget = QApplication::widgetAt(event->where.h, event->where.v);

                //setup the saved widget
                qt_button_down = event->what == mouseDown ? widget : 0;

                //finally send the event to the widget if its not the popup
                if(widget && widget != popupwidget) {
#if 0
                    if(app_do_modal && !qt_try_modal(widget, er))
                        return 1;
#endif
                    if(event->what == mouseDown) {
                        QWidget* w = widget;
                        while(w->focusProxy())
                            w = w->focusProxy();
                        if(w->focusPolicy() & Qt::ClickFocus)
                            w->setFocus(Qt::MouseFocusReason);
                        if(QWidget *tlw = widget->topLevelWidget()) {
                            tlw->raise();
                            if(tlw->isTopLevel() && tlw->windowType() != Qt::Popup &&
                               (tlw->isModal() || tlw->windowType() != Qt::Dialog))
                                QApplication::setActiveWindow(tlw);
                        }
                    }

                    QPoint p(event->where.h, event->where.v);
                    QPoint plocal(widget->mapFromGlobal( p ));
                    QMouseEvent qme(etype, plocal, p, button, 0, keys);
                    QApplication::sendEvent(widget, &qme);
                    return true;
                }
            }
        } else {
            //qDebug("%d", event->what);
        }
    }
    return false;
}

#ifdef QTBROWSER_USE_CFM
static bool qtbrowser_use_cfm = false;
static UInt32 gGlueTemplate[6] = { 0x3D800000, 0x618C0000, 0x800C0000,
                            0x804C0004, 0x7C0903A6, 0x4E800420  };
struct TVector_rec
{
    ProcPtr fProcPtr;
    void *fTOC;
};

void *CFMFunctionPointerForMachOFunctionPointer(void *inMachProcPtr)
{
    if(!qtbrowser_use_cfm)
        return inMachProcPtr;
     TVector_rec *vTVector = (TVector_rec*)malloc(sizeof(TVector_rec));
     if(MemError() == noErr && vTVector != 0) {
         vTVector->fProcPtr = (ProcPtr)inMachProcPtr;
         vTVector->fTOC = 0;  // ignored
     }
     return((void *)vTVector);
}

void DisposeCFMFunctionPointer(void *inCfmProcPtr)
{
    if(!qtbrowser_use_cfm)
        return;
    if(inCfmProcPtr)
        free(inCfmProcPtr);
}

void* MachOFunctionPointerForCFMFunctionPointer(void* inCfmProcPtr)
{
    if(!qtbrowser_use_cfm)
        return inCfmProcPtr;
    UInt32 *vMachProcPtr = (UInt32*)NewPtr(sizeof(gGlueTemplate));
    vMachProcPtr[0] = gGlueTemplate[0] | ((UInt32)inCfmProcPtr >> 16);
    vMachProcPtr[1] = gGlueTemplate[1] | ((UInt32)inCfmProcPtr & 0xFFFF);
    vMachProcPtr[2] = gGlueTemplate[2];
    vMachProcPtr[3] = gGlueTemplate[3];
    vMachProcPtr[4] = gGlueTemplate[4];
    vMachProcPtr[5] = gGlueTemplate[5];
    MakeDataExecutable(vMachProcPtr, sizeof(gGlueTemplate));
    return(vMachProcPtr);
}
#endif


extern "C" void qtns_initialize(QtNPInstance *)
{
    qt_mac_set_native_menubar(false);
    if(!qApp) {
        ownsqapp = true;
        static int argc=0;
        static char **argv={ 0 };
        (void)new QApplication(argc, argv);
    }
}

extern "C" void qtns_destroy(QtNPInstance *This)
{
    delete This->rootWidget;
    This->rootWidget = 0;
}

extern "C" void qtns_shutdown()
{
    if(!ownsqapp)
        return;

    // TODO: find out if other plugin DLL's still need qApp
    delete qApp;
    ownsqapp = false;
}

extern "C" void qtns_embed(QtNPInstance *This)
{
    Q_ASSERT(qobject_cast<QWidget*>(This->qt.object));

    WindowPtr windowptr = GetWindowFromPort((CGrafPtr)This->window->port);
    HIViewRef root = 0;
    OSErr err;
    err = GetRootControl(windowptr,&root);
    if(!root)
        root = HIViewGetRoot(windowptr);
    if(!root) {
        qDebug("No window composition!");
    } else {
        This->rootWidget = new QMacBrowserRoot(root);
        This->qt.widget->setParent(This->rootWidget);
    }
}

extern "C" void qtns_setGeometry(QtNPInstance *This, const QRect &rect, const QRect &clipRect)
{
    Q_ASSERT(qobject_cast<QWidget*>(This->qt.object));

    WindowPtr windowptr = GetWindowFromPort((CGrafPtr)This->window->port);
    Rect content_r;
    GetWindowBounds(windowptr, kWindowContentRgn, &content_r);
    Rect structure_r;
    GetWindowBounds(windowptr, kWindowStructureRgn, &structure_r);

    QRect geom(rect.translated(content_r.left-structure_r.left, content_r.top-structure_r.top));
    if(rect != clipRect) {
        QRegion clipRegion(QRect(clipRect.x()-geom.x(), clipRect.y()-geom.y(), clipRect.width(), clipRect.height())
                           .translated(content_r.left-structure_r.left, content_r.top-structure_r.top));
        if(clipRegion.isEmpty())
            clipRegion = QRegion(-1, -1, 1, 1); //eww ### FIXME
        This->qt.widget->setMask(clipRegion);
    } else {
        This->qt.widget->clearMask();
    }
    This->qt.widget->setGeometry(geom);
}

typedef void (*NPP_ShutdownUPP)(void);

extern "C" void NPP_MacShutdown()
{
    //extern NPError NP_Shutdown();
    //NP_Shutdown();
}

extern "C" int main(NPNetscapeFuncs *npn_funcs, NPPluginFuncs *np_funcs, NPP_ShutdownUPP *shutdown)
{
    qtbrowser_use_cfm = true; //quite the heuristic..

    NPError ret;
    extern NPError NP_Initialize(NPNetscapeFuncs*);
    if((ret=NP_Initialize(npn_funcs)) != NPERR_NO_ERROR)
        return ret;
    extern NPError NP_GetEntryPoints(NPPluginFuncs*);
    if((ret=NP_GetEntryPoints(np_funcs)) != NPERR_NO_ERROR)
        return ret;
    *shutdown = (NPP_ShutdownUPP)MAKE_FUNCTION_POINTER(NPP_MacShutdown);
    return NPERR_NO_ERROR;
}
