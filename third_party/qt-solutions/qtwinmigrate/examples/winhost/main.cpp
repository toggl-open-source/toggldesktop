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
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <qwinhost.h>

#include <windows.h>

class HostWindow : public QWinHost
{
    Q_OBJECT
public:
    HostWindow(QWidget *parent = 0, Qt::WindowFlags f = 0)
	: QWinHost(parent, f)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

    HWND createWindow(HWND parent, HINSTANCE instance)
    {
	static ATOM windowClass = 0;
	if (!windowClass) {
	    WNDCLASSEX wcex;
	    wcex.cbSize		= sizeof(WNDCLASSEX);
	    wcex.style		= CS_HREDRAW | CS_VREDRAW;
	    wcex.lpfnWndProc	= (WNDPROC)WndProc;
	    wcex.cbClsExtra	= 0;
	    wcex.cbWndExtra	= 0;
	    wcex.hInstance	= instance;
	    wcex.hIcon		= NULL;
	    wcex.hCursor	= LoadCursor(NULL, IDC_ARROW);
	    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	    wcex.lpszMenuName	= NULL;
	    wcex.lpszClassName	= L"qtest";
	    wcex.hIconSm	= NULL;

	    windowClass = RegisterClassEx(&wcex);
	}

	HWND hwnd = CreateWindow((TCHAR*)windowClass, 0, WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP, 
	    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, parent, NULL, instance, NULL);

	return hwnd;
    }

signals:
    void message(const QString &msg, int timeout);

public slots:
    void returnPressed()
    {
        QMessageBox::information(topLevelWidget(), "Message from Qt", "Return pressed in QLineEdit!");
    }

protected:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        QWidget *widget = QWidget::find((WId)GetParent(hWnd));
	HostWindow *window = qobject_cast<HostWindow*>(widget);

	if (window) switch (message) {
	case WM_SETFOCUS:
	    window->message("SetFocus for Win32 window!", 1000);
	    break;
	case WM_KILLFOCUS:
	    window->message("KillFocus for Win32 window!", 1000);
	    break;
        case WM_MOUSEMOVE:
            window->message("Moving the mouse, aren't we?", 200);
            break;
	case WM_KEYDOWN:
	    if (wParam != VK_TAB)
		window->message("Key Pressed!", 500);
	    break;
	default:
	    return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
    }
};

#include "main.moc"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QMainWindow mw;
    mw.menuBar()->addMenu("&File")->addAction("&Exit", &a, SLOT(quit()));

    QWidget central(&mw);

    QLineEdit edit(&central);
    HostWindow host(&central);
    QObject::connect(&host, SIGNAL(message(const QString&,int)), mw.statusBar(), SLOT(showMessage(const QString&,int)));
    QObject::connect(&edit, SIGNAL(returnPressed()), &host, SLOT(returnPressed()));

    QVBoxLayout vbox(&central);
    vbox.addWidget(&edit);
    vbox.addWidget(&host);

    mw.setCentralWidget(&central);
    mw.show();
    return a.exec();
}
