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
    return (view_item->Text.toLower().indexOf(filter.toLower()) != -1);
}

void AutocompleteCellWidget::display(AutocompleteView *view) {
    view_item = view;

    // clear styles
    setStyleSheet("");
    ui->label->setStyleSheet("");
    ui->label->setAlignment(Qt::AlignLeft);

    QString transparent = "background-color: transparent;";

    // Format is: Description - TaskName · ProjectName - ClientName

    // Workspace row
    if (view_item->Type == 13) {
        setStyleSheet(transparent + "border-bottom:1px solid grey;");
        ui->label->setText(
                    "<span style='font-weight:bold;font-size:9pt;'>" + view->Text + "</span>");
        ui->label->setAlignment(Qt::AlignCenter);
        return;
    }

    // Category row
    if (view_item->Type == 11) {
        ui->label->setStyleSheet(transparent + "padding-top:7px;padding-left:5px;font-size:9pt;");
        ui->label->setText(view->Text);
        return;
    }

    // Client row / no project row
    if (view_item->Type == 12 || (view_item->Type == 2 && view_item->ProjectID == 0)) {
        QString style = transparent + "padding-top:5px;padding-left:10px;font-size:9pt;font-weight:";
        if (view_item->Type == 2) {
            style.append("normal;");
        } else {
            style.append("800;");
        }
        ui->label->setStyleSheet(style);
        ui->label->setText(view->Text);
        return;
    }

    // Task row
    if (view_item->Type == 1)
    {
        ui->label->setStyleSheet(transparent + "padding-top:8px;padding-left:30px;font-size:9pt;");
        ui->label->setText("- " + view_item->Text);
        return;
    }

    // Item rows (projects/time entries)
    ui->label->setStyleSheet(transparent + "padding-left:15px;font-size:9pt;");

    QString text = QString(view->Description);
    if (view_item->ProjectID != 0)
    {
        if (view_item->TaskID != 0)
        {
            text.append(QString(" - " + view_item->TaskLabel));
        }
        ui->label->setStyleSheet(transparent + "padding-left:15px;font-size:9pt;");
        text.append(QString(" <span style='font-size:20px;color:" +
                       view->ProjectColor + ";'> •</span> " +
                       view->ProjectLabel));
    } else {
        ui->label->setStyleSheet(transparent + "padding-top:7px;padding-left:15px;font-size:9pt;");
    }

    // Add client label to time entry items
    if (view_item->Type == 0) {
        text.append(QString("  <span style='font-weight:800;'>" + view_item->ClientLabel + "</span>"));
    }

    ui->label->setText(text);
}

