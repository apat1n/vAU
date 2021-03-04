#include "mainwindow.h"
#include <iostream>
#include "ui_mainwindow.h"
#include "utils.cpp"

MainWindow::MainWindow(const QString &server_url, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      client(server_url),
      createChat(new QListWidgetItem("Create new chat")) {
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->errorLogin->hide();
    ui->errorRegister->hide();
    ui->labelUnstableConnection->hide();
    client.connectServer();
    ui->inputPassword->setEchoMode(QLineEdit::Password);

    connect(&client, &Client::connectionUnstable, this,
            &MainWindow::onConnectionUnstable);

    availableChats.append(new Chat(0, "Artificial chat"));
    availableChats.append(new Chat(1, "Bartificial chat 2"));
    ui->chatView->hide();
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
        // Creating message
        Message *message = new Message(messageText, "Me");
        message->setText(messageText);
        //

        // Sending to db and adding to listview
        if (Chat *chat = dynamic_cast<Chat *>(ui->chatMenu->currentItem());
            chat) {
            client.sendMessage(messageText);
            chat->addMessage(message);
            ui->chatView->addItem(dynamic_cast<QListWidgetItem *>(message));
            qDebug() << chat->getHistory().size();
        }

        ui->messageTextField->clear();
    }
}

void MainWindow::on_search_textEdited(const QString &searchRequest) {
    if (!searchRequest.isEmpty()) {
        renderChats(foundMatches(availableChats, searchRequest));
    } else {
        renderChats(availableChats);
    }
}

void MainWindow::renderChats(const QList<Chat *> &chatsList) {
    clearListWidget(ui->chatMenu);
    ui->chatMenu->addItem(createChat);
    for (auto it : chatsList) {
        QListWidgetItem *item = it;
        item->setText(it->getName());
        ui->chatMenu->addItem(it);
    }
}

void MainWindow::renderMessages(Chat *chat) {
    clearListWidget(ui->chatView);
    for (auto it : chat->getHistory()) {
        if (it) {
            if (QListWidgetItem *widgetElem =
                    dynamic_cast<QListWidgetItem *>(it);
                widgetElem) {
                ui->chatView->addItem(widgetElem);
            }
        }
        // TODO: try - catch
    }
}

void MainWindow::on_chatMenu_itemClicked(QListWidgetItem *item) {
    if (item == createChat) {
        if (client.createChat("test_chat")) {
            availableChats.append(new Chat(1, "test_chat"));
        }
    } else {
        Chat *chat = dynamic_cast<Chat *>(item);
        if (chat) {
            ui->chatView->show();
            renderMessages(chat);
        }
    }
}
