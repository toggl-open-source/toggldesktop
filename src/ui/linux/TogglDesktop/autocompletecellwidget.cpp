#include "autocompletecellwidget.h"
#include "ui_autocompletecellwidget.h"
#include <QDebug>

AutocompleteCellWidget::AutocompleteCellWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutocompleteCellWidget)
{
    ui->setupUi(this);
}

AutocompleteCellWidget::~AutocompleteCellWidget()
{
    delete ui;
}

bool AutocompleteCellWidget::filter(QString filter) {
    if (view_item->Text.toLower().indexOf(filter.toLower()) != -1)
        qDebug() << "FF: " << view_item->Text << " -- " << filter << " == " << view_item->Text.indexOf(filter);
    return (view_item->Text.toLower().indexOf(filter.toLower()) != -1);
}

void AutocompleteCellWidget::display(AutocompleteView *view) {
    view_item = view;

    // clear styles
    setStyleSheet("");
    ui->label->setStyleSheet("");

    // Format is: Description - TaskName · ProjectName - ClientName

    // Workspace row
    if (view_item->Type == 13) {
        setStyleSheet("border-bottom:1px solid grey;");
        ui->label->setStyleSheet("text-align: center;");
        ui->label->setText(
                    "<span style='text-align:center;font-weight:bold;font-size:9pt;'>" + view->Text + "</span>");
        return;
    }

    // Category row
    if (view_item->Type == 11) {
        ui->label->setStyleSheet("padding-left:5px;font-size:9pt;");
        ui->label->setText(view->Text);
        return;
    }

    // Client row / no project row
    if (view_item->Type == 12 || (view_item->Type == 2 && view_item->ProjectID == 0)) {
        ui->label->setStyleSheet("padding-left:10px;font-size:9pt;");
        ui->label->setText(view->Text);
        return;
    }

    // Task row
    if (view_item->TaskID != 0)
    {
        ui->label->setStyleSheet("padding-left:30px;font-size:9pt;");
        ui->label->setText(view->Text);
        return;
    }

    // Item rows (projects/time entries)
    QString text = QString(view->Description);
    if (view_item->ProjectID != 0)
    {
        ui->label->setStyleSheet("padding-left:15px;font-size:9pt;");
        text.append(QString(" <span style='font-size:20px;color:" +
                       view->ProjectColor + ";'> •</span> " +
                       view->ProjectLabel));
    } else {
        ui->label->setStyleSheet("padding-left:10px;font-size:9pt;");
    }

    ui->label->setText(text);
}

