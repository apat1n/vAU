#include "creatingChat.h"
#include "ui_creatingChat.h"

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog) {
    ui->setupUi(this);
}

Dialog::~Dialog() {
    delete ui;
}

void Dialog::on_createChat_clicked() {
    QString name = ui->chatName->text();
    if (name != "") {
        emit requestCreating(name);
        this->close();
    }
}
