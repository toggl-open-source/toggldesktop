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
    qDebug() << "Cell Disaply: " << view->Text;
    ui->label->setText(view->Text);
    //setEllipsisTextToLabel(ui->label, view->Text);
}

void AutocompleteCellWidget::setEllipsisTextToLabel(QLabel *label, QString text)
{
    QFontMetrics metrix(label->font());
    int width = label->width() - 2;
    QString clippedText = metrix.elidedText(text, Qt::ElideRight, width);
    label->setText(clippedText);
}
