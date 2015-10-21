// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_SINGLEAPPLICATION_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_SINGLEAPPLICATION_H_

#include <QObject>
#include <QApplication>
#include <QtNetwork/QLocalServer>
#include <QWidget>

class SingleApplication : public QApplication {
    Q_OBJECT
 public:
    SingleApplication(int &argc, char **argv);  // NOLINT

    bool isRunning();
    QWidget *w;

 private slots:  // NOLINT
    void newLocalConnection();

 private:
    void initLocalConnection();
    void newLocalServer();
    void activateWindow();

    bool is_running_;
    QLocalServer *local_server_;
    QString server_name_;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_SINGLEAPPLICATION_H_
