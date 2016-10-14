// Copyright 2016 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_COLORPICKER_H
#define SRC_UI_LINUX_TOGGLDESKTOP_COLORPICKER_H

#include <QDialog>
#include <QVector>

namespace Ui {
class ColorPicker;
}

class ColorPicker : public QDialog
{
    Q_OBJECT

public:
    explicit ColorPicker(QWidget *parent = 0);
    ~ColorPicker();

private:
    Ui::ColorPicker *ui;

public:
    void setColors(QVector<char *> list);

private slots:
    void color_clicked();
    void on_closeButton_clicked();
};

#endif // SRC_UI_LINUX_TOGGLDESKTOP_COLORPICKER_H
