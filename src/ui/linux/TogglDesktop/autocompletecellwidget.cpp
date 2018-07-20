#include "autocompletecellwidget.h"
#include "ui_autocompletecellwidget.h"

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

void AutocompleteCellWidget::display(AutocompleteView *view) {
    view_item = view;

    // clear styles
    setStyleSheet("");
    ui->label->setStyleSheet("");

    // Format is: Description - TaskName · ProjectName - ClientName


    // Workspace row
    if (view_item->Type == 13) {
        setStyleSheet("border-bottom:1px solid grey;");
        ui->label->setStyleSheet("font-weight:bold; text-align: center;");
        ui->label->setText(view->Text);
        return;
    }

    // Category row
    if (view_item->Type == 11) {
        //ui->label->setStyleSheet("margin-left:10px;");
        ui->label->setText(view->Text);
        return;
    }

    // Client row / no project row
    if (view_item->Type == 12 || (view_item->Type == 2 && view_item->ProjectID == 0)) {
        ui->label->setStyleSheet("margin-left:10px;");
        ui->label->setText(view->Text);
        return;
    }

    // Task row
    if (view_item->TaskID != 0)
    {
        ui->label->setStyleSheet("margin-left:30px;");
        ui->label->setText(view->Text);
        return;
    }

    // Item rows (projects/time entries)
    QString text = QString(view->Description);
    if (view_item->ProjectID != 0)
    {
        ui->label->setStyleSheet("margin-left:20px;");
        text.append(QString(" <span style='font-size:20px;color:" +
                       view->ProjectColor + ";'>•</span> " +
                       view->ProjectLabel));
    }

    ui->label->setText(text);
}

