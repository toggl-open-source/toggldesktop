/********************************************************************************
** Form generated from reading UI file 'loginviewcontroller.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINVIEWCONTROLLER_H
#define UI_LOGINVIEWCONTROLLER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginViewController
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *LoginViewController)
    {
        if (LoginViewController->objectName().isEmpty())
            LoginViewController->setObjectName(QStringLiteral("LoginViewController"));
        LoginViewController->resize(400, 300);
        verticalLayoutWidget = new QWidget(LoginViewController);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 20, 160, 80));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);

        retranslateUi(LoginViewController);

        QMetaObject::connectSlotsByName(LoginViewController);
    } // setupUi

    void retranslateUi(QWidget *LoginViewController)
    {
        LoginViewController->setWindowTitle(QApplication::translate("LoginViewController", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class LoginViewController: public Ui_LoginViewController {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINVIEWCONTROLLER_H
