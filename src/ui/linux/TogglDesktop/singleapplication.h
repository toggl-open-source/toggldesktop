#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QObject>
#include <QApplication>
#include <QtNetwork/QLocalServer>
#include <QWidget>

class SingleApplication : public QApplication {
    Q_OBJECT
 public:
    SingleApplication(int &argc, char **argv);

    bool isRunning();
    QWidget *w;

 private slots:
    void newLocalConnection();

 private:
    void initLocalConnection();
    void newLocalServer();
    void activateWindow();

    bool is_running_;
    QLocalServer *local_server_;
    QString server_name_;
};

#endif // SINGLEAPPLICATION_H
