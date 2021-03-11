#include "mainwindow.h"
#include <iostream>
#include <utility>
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
    ui->chatView->hide();
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
        // client.sendMessage(inputText);
        ui->inputText->clear();
    }
}

void MainWindow::on_signIn_clicked() {
    QString login = ui->inputLogin->text();
    QString password = ui->inputPassword->text();

    if (client.loginUser(login, password)) {
        ui->stackedWidget->setCurrentIndex(2);
        updateChats();
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
        updateChats();
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
        Message *message = new Message(messageText, client.getId());
        message->setText(messageText);
        //

        // Sending to db and adding to listview
        if (Chat *chat = dynamic_cast<Chat *>(ui->chatMenu->currentItem());
            chat) {
            if (client.sendMessage(message, chat->getChatId())) {
                qDebug() << "Successfully sent!";
                renderMessages(chat);
            }
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
    client.getUserList(availibleUsers);
    clearListWidget(ui->chatMenu);
    ui->chatMenu->addItem(createChat);
    for (auto it : chatsList) {
        QListWidgetItem *item = it;
        item->setText(it->getName());
        ui->chatMenu->addItem(it);
    }
}

void MainWindow::renderMessages(Chat *chat) {
    client.getUserList(availibleUsers);
    QList<Message *> newHistory;
    if (client.getChatMessages(chat->getChatId(), newHistory)) {
        clearListWidget(ui->chatView);
        chat->updateMessageHistory(std::move(newHistory));
        qDebug() << "Got " << chat->getHistory().size() << " messages!";
        for (auto it : chat->getHistory()) {
            if (it) {
                qDebug() << it->getText();
                if (QListWidgetItem *widgetElem =
                        dynamic_cast<QListWidgetItem *>(it);
                    widgetElem) {
                    QString textField = "[ " +
                                        availibleUsers[it->getAuthorId()] +
                                        " ] : " + it->getText();
                    qDebug() << textField;
                    widgetElem->setText(textField);
                    ui->chatView->addItem(widgetElem);
                }
            }
            // TODO: try - catch
        }
    }
}

void MainWindow::on_chatMenu_itemClicked(QListWidgetItem *item) {
    if (item == createChat) {
        ui->chatView->hide();
        if (client.createChat("test_chat")) {
            updateChats();
        }
    } else {
        Chat *chat = dynamic_cast<Chat *>(item);
        if (chat) {
            ui->chatView->show();
            renderMessages(chat);
        }
    }
}

void MainWindow::updateChats() {
    QList<Chat *> newAvailableChats;
    if (client.getChatList(newAvailableChats)) {
        clearListWidget(ui->chatMenu);
        availableChats.clear();
        std::swap(newAvailableChats, availableChats);
    }
    renderChats(availableChats);
}
