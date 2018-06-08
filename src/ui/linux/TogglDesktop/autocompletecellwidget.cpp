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


void AutocompleteCellWidget::display(AutocompleteView *view) {
    QString text("");
    // project
    if (view->Type == 2) {
        qDebug()<<"COLOR: "<< view->ProjectColor << " | " << view->ProjectLabel;
        text.append("<p><span style='margin:0;padding:0;display:block;width:5px;height:5px;font-size:30px;margin-right:10px;color:");
        text.append(view->ProjectColor);
        text.append("'> • </span><span style='background-color:red'>");
        text.append(view->ProjectLabel);
        text.append("</span></p>");
    }
    qDebug() << "Cell hDisaply: " << view->Text << " || " << view->Type;
    qDebug() << text;

    ui->label->setText(text);
    //setEllipsisTextToLabel(ui->label, view->Text);
}

void AutocompleteCellWidget::setEllipsisTextToLabel(QLabel *label, QString text)
{
    QFontMetrics metrix(label->font());
    int width = label->width() - 2;
    QString clippedText = metrix.elidedText(text, Qt::ElideRight, width);
    label->setText(clippedText);
}
