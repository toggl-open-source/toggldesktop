#include "autocompletedropdown.h"
#include "ui_autocompletedropdown.h"
#include <QDebug>

AutocompleteDropdown::AutocompleteDropdown(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutocompleteDropdown)
{
    ui->setupUi(this);

    setModal(false);
    setWindowTitle("");
    setWindowFlags(Qt::ToolTip);

    // Add shadow
    setAttribute(Qt::WA_TranslucentBackground); //enable MainWindow to be transparent

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setOffset(0);
    effect->setBlurRadius(5);
    effect->setYOffset(3);
    ui->list->setGraphicsEffect(effect);
}

AutocompleteDropdown::~AutocompleteDropdown()
{
    delete ui;
}

void AutocompleteDropdown::reload(QVector<AutocompleteView *> list){
    qDebug() << "AutocompleteDropdown::reload";
    int size = list.size();
    qDebug() << "LIst ssize [" << size << "]";

    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    for (int i = 0; i < size; i++) {
        AutocompleteView *a = list.at(i);

        QListWidgetItem *item = 0;
        AutocompleteCellWidget *cell = 0;

        if (ui->list->count() > i) {
            qDebug() << ui->list->count() << " > " << i;
            item = ui->list->item(i);
            cell = static_cast<AutocompleteCellWidget *>(
                ui->list->itemWidget(item));
        }

        if (!item) {
            item = new QListWidgetItem();
            cell = new AutocompleteCellWidget();

            ui->list->addItem(item);
            ui->list->setItemWidget(item, cell);
        }

        cell->display(a);
    }

    while (ui->list->count() > size) {
        ui->list->model()->removeRow(size);
    }

    setVisible(!list.isEmpty());
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void AutocompleteDropdown::onKeyPressEvent(QKeyEvent* event) {
    QDialog::keyPressEvent(event);
}
