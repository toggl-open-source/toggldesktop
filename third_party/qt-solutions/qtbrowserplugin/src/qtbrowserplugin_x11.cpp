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

#include <QtGui>
#include <QX11EmbedWidget>

#include "qtbrowserplugin.h"
#include "qtbrowserplugin_p.h"

#include "qtnpapi.h"

#include <stdlib.h>

static bool ownsqapp = false;
static QMap<QtNPInstance*, QX11EmbedWidget*> clients;

extern "C" bool qtns_event(QtNPInstance *, NPEvent *)
{
    return false;
}

extern "C" void qtns_initialize(QtNPInstance* This)
{
    if (!qApp) {
        ownsqapp = true;
        static int argc = 0;
        static char **argv = {0};

        // Workaround to avoid re-initilaziation of glib
        char* envvar = qstrdup("QT_NO_THREADED_GLIB=1");
        // Unavoidable memory leak; the variable must survive plugin unloading
        ::putenv(envvar);

        (void)new QApplication(argc, argv);
    }
    if (!clients.contains(This)) {
        QX11EmbedWidget* client = new QX11EmbedWidget;
        QHBoxLayout* layout = new QHBoxLayout(client);
        layout->setMargin(0);
        clients.insert(This, client);
    }
}

extern "C" void qtns_destroy(QtNPInstance* This)
{
    QMap<QtNPInstance*, QX11EmbedWidget*>::iterator it = clients.find(This);
    if (it == clients.end())
        return;
    delete it.value();
    clients.erase(it);
}

extern "C" void qtns_shutdown()
{
    if (clients.count() > 0) {
        QMap<QtNPInstance*, QX11EmbedWidget*>::iterator it = clients.begin();
        while (it != clients.end()) {
            delete it.value();
            ++it;
        }
        clients.clear();
    }

    if (!ownsqapp)
        return;

    // check if qApp still runs widgets (in other DLLs)
    QWidgetList widgets = qApp->allWidgets();
    int count = widgets.count();
    for (int w = 0; w < widgets.count(); ++w) {
        // ignore all Qt generated widgets
        QWidget *widget = widgets.at(w);
        if (widget->windowFlags() & Qt::Desktop)
            count--;
    }
    if (count) // qApp still used
        return;

    delete qApp;
    ownsqapp = false;
}

extern "C" void qtns_embed(QtNPInstance *This)
{
    Q_ASSERT(qobject_cast<QWidget*>(This->qt.object));

    QMap<QtNPInstance*, QX11EmbedWidget*>::iterator it = clients.find(This);
    if (it == clients.end())
        return;
    QX11EmbedWidget* client = it.value();
    This->qt.widget->setParent(client);
    client->layout()->addWidget(This->qt.widget);
    client->embedInto(This->window);
    client->show();
}

extern "C" void qtns_setGeometry(QtNPInstance *This, const QRect &rect, const QRect &)
{
    Q_ASSERT(qobject_cast<QWidget*>(This->qt.object));

    QMap<QtNPInstance*, QX11EmbedWidget*>::iterator it = clients.find(This);
    if (it == clients.end())
        return;
    QX11EmbedWidget* client = it.value();
    client->setGeometry(QRect(0, 0, rect.width(), rect.height()));
}

/*
extern "C" void qtns_print(QtNPInstance * This, NPPrint *printInfo)
{
    NPWindow* printWindow = &(printInfo->print.embedPrint.window);
    void* platformPrint = printInfo->print.embedPrint.platformPrint;
    // #### Nothing yet.
}
*/
