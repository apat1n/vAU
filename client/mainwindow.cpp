#include "mainwindow.h"
#include <iostream>
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString &server_url, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), client(server_url) {
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->errorLogin->hide();
    ui->errorRegister->hide();
    ui->labelUnstableConnection->hide();
    client.connectServer();
    ui->inputPassword->setEchoMode(QLineEdit::Password);

    connect(&client, &Client::connectionUnstable, this,
            &MainWindow::onConnectionUnstable);

    availableChats.append(new Chat(-1, "Create new chat"));
    availableChats.append(new Chat(0, "Artificial chat"));
    renderChats(availableChats);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onConnectionUnstable() {
    ui->labelUnstableConnection->show();
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
        ui->stackedWidget->setCurrentIndex(2);
        ui->errorLogin->hide();
    } else {
        ui->errorLogin->show();
    }
    ui->inputPassword->clear();
}

void MainWindow::on_signOut_clicked() {
    if (client.logoutUser()) {
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

void MainWindow::on_messageTextField_returnPressed() {
    QString messageText = ui->messageTextField->text();
    if (!messageText.isEmpty()) {
        client.sendMessage(messageText);
        // if ok
        QListWidgetItem *item = new Message(messageText, "Me");
        item->setText(messageText);
        ui->chatView->addItem(item);

        ui->messageTextField->clear();
    }
}

void MainWindow::on_search_textEdited(const QString &searchRequest) {
    //    if (!searchRequest.isEmpty()){
    //        // some actions with database
    //        qDebug("before search");
    //        QList<QListWidgetItem*> results =
    //        availableChats.findItems(searchRequest, Qt::MatchStartsWith);
    //        qDebug("after search");
    //        ui->chatMenu->clear();
    //        for (auto it: results) {
    //            ui->chatMenu->addItem(it);
    //        }

    //    }
    //    else {
    //        renderChats(availableChats);
    //    }
}

void MainWindow::renderChats(const QList<Chat *> &chatsList) {
    ui->chatMenu->clear();
    for (auto it : chatsList) {
        QListWidgetItem *item = it;
        item->setText(it->getName());
        ui->chatMenu->addItem(it);
    }
}

void MainWindow::renderMessages(Chat *chat) {
    ui->chatView->clear();
    for (auto it : chat->getHistory()) {
        QListWidgetItem *item = it;
        item->setText(it->getText());
        ui->chatView->addItem(item);
    }
}

void MainWindow::on_chatMenu_itemClicked(QListWidgetItem *item) {
    Chat *chat = dynamic_cast<Chat *>(item);
    if (chat->getChatId() == -1) {
        client.createChat("test_chat");
    }
}
