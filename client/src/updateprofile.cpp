#include "updateprofile.h"
#include "client.h"
#include "client_processes.cpp"
#include "ui_updateprofile.h"

updateProfile::updateProfile(QString login_, QString status_, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::updateProfile),
      login(login_),
      status(status_) {
    ui->setupUi(this);
    ui->newLoginEdit->setText(login);
    ui->newStatusEdit->setText(status);
    this->setWindowTitle("Change your profile");
}

updateProfile::~updateProfile() {
    delete ui;
}

void updateProfile::on_changeProfile_clicked() {
    if (ui->newLoginEdit->text() != login) {
        emit requestChangeLogin(ui->newLoginEdit->text());
    }
    if (ui->newStatusEdit->text() != status) {
        emit requestChangeStatus(ui->newStatusEdit->text());
    }
    this->close();
}
