#include "createChat.h"
#include "ui_createChat.h"
#include "client.h"
#include <QListWidget>
#include <QtCore/QDebug>
Dialog::Dialog(QMap<int,QString> users_, QWidget *parent) : QDialog(parent), ui(new Ui::Dialog){

    ui->setupUi(this);
    QMap<int, QString>::iterator i;
    for (i=users_.begin();i!=users_.end();i++){
        QListWidgetItem *it = new QListWidgetItem;
        it->setText(i.value());
        it->setCheckState(Qt::Unchecked);
        it->setData(Qt::UserRole,i.key());
//        it->setIcon(QIcon("/home/vtgcon/Загрузки/aa88d9c40c496abf8d09897c1816082ee0a69817.jpeg"));
//  так я буду ставить иконки юзеров, можете раскоментить и проверить своей фоткой, что все работает)
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
            if (ui->avaliableUsers->item(i)->checkState() == Qt::Checked){
                emit requestAddUser(ui->avaliableUsers->item(i)->data(Qt::UserRole).toInt());
            }
        }
        emit requestCreating(name);
        this->close();
    }
}

//void Dialog::on_avaliableUsers_itemClicked(QListWidgetItem *item)
//{
//    QListWidgetItem *it = new QListWidgetItem();
//    it->setText(item->text());
//    int a = item->data(Qt::UserRole).toInt();
//    qDebug() << a;
//    it->setData(Qt::UserRole,a);
//    bool flag = false;
//    for (int i = 0; i < ui->invitedUsers->count(); i++){
//        if (ui->invitedUsers->item(i)->data(Qt::UserRole).toInt()==a){
//            flag = true;
//        }
//    }
//    if (flag == false){
//       ui->invitedUsers->addItem(it);
//    } else {
//        delete it;
//    }
//}
