#include "createChat.h"
#include <QListWidget>
#include <QtCore/QDebug>
#include "client.h"
#include "ui_createChat.h"
#include "utils.cpp"

Dialog::Dialog(Client &client, QMap<int, QString> users_, QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), client(client) {
    ui->setupUi(this);
    ui->avaliableUsers->setIconSize(QSize(32, 32));
    QMap<int, QString>::iterator i;
    for (i = users_.begin(); i != users_.end(); i++) {
        QListWidgetItem *it = new QListWidgetItem;
        it->setText(i.value());
        it->setCheckState(Qt::Unchecked);
        it->setData(Qt::UserRole, i.key());
        it->setIcon(QPixmap::fromImage(getUserImage(i.key(), client)));
        ui->avaliableUsers->addItem(it);
    }
}

Dialog::~Dialog() {
    delete ui;
}

void Dialog::on_createChat_clicked() {
    QString name = ui->chatName->text();
    if (name != "") {
        for (int i = 0; i < ui->avaliableUsers->count(); i++) {
            if (ui->avaliableUsers->item(i)->checkState() == Qt::Checked) {
                emit requestAddUser(
                    ui->avaliableUsers->item(i)->data(Qt::UserRole).toInt());
            }
        }
        emit requestCreating(name);
        this->close();
    }
}

void Dialog::on_avaliableUsers_itemClicked(QListWidgetItem *item) {
}
