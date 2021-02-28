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
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateDebugText(const QString &message) {
    QString debugPlainText = ui->debugText->toPlainText();
    ui->debugText->setText(debugPlainText + message + "\n");
}

void MainWindow::on_connectButton_clicked() {
    client.connectServer();
    updateDebugText("CONNECTED");
}

void MainWindow::on_DisconnectButton_clicked() {
    client.disconnectServer();
    updateDebugText("DISCONNECTED");
}

void MainWindow::on_StateButton_clicked() {
    updateDebugText(client.getState());
}

void MainWindow::on_SendButton_clicked() {
    if (!ui->inputText->document()->isEmpty()) {
        QString inputText = ui->inputText->toPlainText();
        client.sendMessage(inputText);
        ui->inputText->clear();
    }
}

void MainWindow::on_signIn_clicked() {
    QString login = ui->inputLogin->toPlainText();
    QString password = ui->inputPassword->toPlainText();

    if (client.loginUser(login, password)) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->errorLogin->hide();
    } else {
        ui->errorLogin->show();
    }
    ui->inputPassword->clear();
}

void MainWindow::on_registerButton_clicked() {
    QString login = ui->inputLogin->toPlainText();
    QString password = ui->inputPassword->toPlainText();

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
