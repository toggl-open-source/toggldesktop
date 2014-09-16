// Copyright 2014 Toggl Desktop developers.

#include "./clickablelabel.h"
#include "./toggl.h"
#include "./timeentrycellwidget.h"

ClickableLabel::ClickableLabel(QWidget * parent) : QLabel(parent) {
}

ClickableLabel::~ClickableLabel() {
}

void ClickableLabel::mousePressEvent(QMouseEvent * event) {
    QWidget* parentObject = this->parentWidget();

    while (parentObject->objectName().compare("TimeEntryCellWidget") != 0) {
        parentObject = parentObject->parentWidget();
    }

    TimeEntryCellWidget *Cell =
        qobject_cast<TimeEntryCellWidget *>(parentObject);
    Cell->labelClicked(this->objectName());
}
