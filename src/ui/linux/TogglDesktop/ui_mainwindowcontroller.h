/********************************************************************************
** Form generated from reading UI file 'mainwindowcontroller.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOWCONTROLLER_H
#define UI_MAINWINDOWCONTROLLER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowController
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowController)
    {
        if (MainWindowController->objectName().isEmpty())
            MainWindowController->setObjectName(QStringLiteral("MainWindowController"));
        MainWindowController->resize(301, 420);
        MainWindowController->setMinimumSize(QSize(300, 420));
        centralWidget = new QWidget(MainWindowController);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindowController->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowController);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 301, 25));
        MainWindowController->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowController);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowController->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowController);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowController->setStatusBar(statusBar);

        retranslateUi(MainWindowController);

        QMetaObject::connectSlotsByName(MainWindowController);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowController)
    {
        MainWindowController->setWindowTitle(QApplication::translate("MainWindowController", "Toggl Desktop", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowController: public Ui_MainWindowController {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOWCONTROLLER_H
