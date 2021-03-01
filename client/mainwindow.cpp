#include "mainwindow.h"
#include <iostream>
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString &server_url, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), client(server_url) {
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->errorLogin->hide();
    ui->errorRegister->hide();
    client.connectServer();
    ui->inputPassword->setEchoMode(QLineEdit::Password);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_SendButton_clicked() {
    if (!ui->inputText->document()->isEmpty()) {
        QString inputText = ui->inputText->toPlainText();
        client.sendMessage(inputText);
        ui->inputText->clear();
    }
}

void MainWindow::on_signIn_clicked() {
    QString login = ui->inputLogin->text();
    QString password = ui->inputPassword->text();

    if (client.loginUser(login, password)) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->errorLogin->hide();
    } else {
        ui->errorLogin->show();
    }
    ui->inputPassword->clear();
}

void MainWindow::on_signOut_clicked() {
    if (client.logoutUser()){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_registerButton_clicked() {
    QString login = ui->inputLogin->text();
    QString password = ui->inputPassword->text();

    if (client.registerUser(login, password)) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->errorRegister->hide();
    } else {
        ui->errorRegister->show();
    }
}

void MainWindow::on_pushButton_2_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}
