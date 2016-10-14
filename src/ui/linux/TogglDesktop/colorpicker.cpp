// Copyright 2016 Toggl Desktop developers.

#include <QGraphicsDropShadowEffect>
#include "colorpicker.h"
#include "ui_colorpicker.h"
#include "timeentryeditorwidget.h"

ColorPicker::ColorPicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorPicker)
{
    ui->setupUi(this);
    setModal(true);
    setWindowTitle("Color Select");
    setWindowFlags( Qt::Popup );

    // Add shadow
    setAttribute(Qt::WA_TranslucentBackground); //enable MainWindow to be transparent

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setOffset(0);
    effect->setBlurRadius(5);
    ui->verticalWidget->setGraphicsEffect(effect);
}

ColorPicker::~ColorPicker()
{
    delete ui;
}

void ColorPicker::setColors(QVector<char *> list) {
    int row = 0;
    int cell = 0;

    for (int i = 0; i < list.size(); i++) {
        QPushButton* colorButton = new QPushButton(this);
        colorButton->setText(QString("■"));
        colorButton->setMinimumSize(QSize(20, 20));
        colorButton->setMaximumSize(QSize(20, 20));
        colorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QString style = "color:" + QString::fromUtf8(list.at(i)) + ";";
        colorButton->setStyleSheet(style);
        connect(colorButton,SIGNAL(clicked()),this,SLOT(color_clicked()));

        ui->gridLayout->addWidget(colorButton,row,cell);

        cell++;

        // Keep 5 items in 1 row
        if ((i+1) % 5 == 0) {
            row++;
            cell = 0;
        }
    }
}

void ColorPicker::color_clicked() {
    QString color = ((QPushButton*)sender())->styleSheet();
    TimeEntryEditorWidget *p =(TimeEntryEditorWidget*)parent();
    p->setSelectedColor(color);
    close();
}

void ColorPicker::on_closeButton_clicked()
{
    close();
}
