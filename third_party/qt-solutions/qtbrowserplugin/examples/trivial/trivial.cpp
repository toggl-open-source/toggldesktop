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
#include <qtbrowserplugin.h>

class Trivial : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)

    Q_CLASSINFO("ClassID", "{5a22176d-118f-4185-9653-9f98958a6df8}")
    Q_CLASSINFO("InterfaceID", "{2df735ba-da4f-4fb7-8f35-b8dfbf8cfd9a}")
    Q_CLASSINFO("EventsID", "{449de213-f8bd-4d2e-a2cf-eab407c03245}")

    Q_CLASSINFO("MIME", "trivial/very:xxx:Trivial and useless")
    Q_CLASSINFO("ToSuperClass", "Trivial")
    Q_CLASSINFO("DefaultProperty", "text")

public:
    Trivial(QWidget *parent = 0)
        : QWidget(parent)
    {
        m_text = QString::fromLatin1("Empty");
    }

    void mousePressEvent(QMouseEvent *)
    {
        mouseDown();
    }
    void mouseMoveEvent(QMouseEvent *e)
    {
        mouseMove(e->x(), e->y());
    }
    void mouseReleaseEvent(QMouseEvent*)
    {
        mouseUp();
    }
    QString text() const
    {
        return m_text;
    }

public slots:
    void about()
    {
        QMessageBox::aboutQt(this);
    }
    void setText(const QString &text)
    {
        m_text = text;
        update();
    }

signals:
    void mouseDown();
    void mouseMove(int x, int y);
    void mouseUp();

protected:
    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        QRect r(rect());
        r.adjust(0, 0, -1, -1);

        p.drawRect(r);
        p.drawText(r, Qt::AlignCenter, m_text);
    }

private:
    QString m_text;
};

#include "trivial.moc"

QTNPFACTORY_BEGIN("Trivial Qt-based Plugin", "A Qt-based LiveConnected plug-in that does nothing")
    QTNPCLASS(Trivial)
QTNPFACTORY_END()

#ifdef QAXSERVER
#include <ActiveQt/QAxFactory>
QAXFACTORY_BEGIN("{aa3216bf-7e20-482c-84c6-06167bacb616}", "{08538ca5-eb7a-4f24-a3c4-a120c6e04dc4}")
    QAXCLASS(Trivial)
QAXFACTORY_END()
#endif
