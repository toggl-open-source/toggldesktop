// Copyright 2014 Toggl Desktop developers.

#include "./clickablelabel.h"
#include "./toggl.h"
#include "./timeentrycellwidget.h"
#include "./autocompletecellwidget.h"

ClickableLabel::ClickableLabel(QWidget * parent) : QLabel(parent) {
    type = 0;
}

ClickableLabel::~ClickableLabel() {
}

void ClickableLabel::mousePressEvent(QMouseEvent * event) {
    QWidget* parentObject = this->parentWidget();

    if (type == 0) {

        while (parentObject->objectName().compare("TimeEntryCellWidget") != 0) {
            parentObject = parentObject->parentWidget();
        }

        TimeEntryCellWidget *Cell =
            qobject_cast<TimeEntryCellWidget *>(parentObject);
        Cell->labelClicked(this->objectName());

    } else if (type == 1) {
        while (parentObject->objectName().compare("AutocompleteCellWidget") != 0) {
            parentObject = parentObject->parentWidget();
        }

        AutocompleteCellWidget *Cell =
            qobject_cast<AutocompleteCellWidget *>(parentObject);
        Cell->labelClicked(this->objectName());
    }

}
