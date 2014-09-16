// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_CLICKABLELABEL_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_CLICKABLELABEL_H_

#include <QLabel>
#include <QDebug>

class ClickableLabel : public QLabel {
 public:
    explicit ClickableLabel(QWidget * parent = 0);
    ~ClickableLabel();

 signals:
    void clicked();

 protected:
    void mousePressEvent(QMouseEvent * event);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_CLICKABLELABEL_H_
