#include "mainwindow.h"
#include <iostream>
#include <memory>
#include <utility>
#include "createChat.h"
#include "ui_mainwindow.h"
#include "utils.cpp"

MainWindow::MainWindow(const QString &server_url, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), client(server_url) {
    ui->setupUi(this);
    ui->errorLogin->hide();
    ui->errorRegister->hide();
    ui->labelUnstableConnection->hide();
    client.connectServer();
    ui->inputPassword->setEchoMode(QLineEdit::Password);

    connect(&client, &Client::connectionUnstable, this,
            &MainWindow::onConnectionUnstable);
    connect(&client, &Client::responsePushMessageReceived,
            [this](int chat_id) { renderMessages(chat_id); });

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
        ui->stackedWidget->setCurrentIndex(2);
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
        Message *raw_message = new Message(messageText, client.getId());
        raw_message->setText(messageText);
        QSharedPointer<Message> message(raw_message);
        //

        // Sending to db and adding to listview
        if (Chat *chat = dynamic_cast<Chat *>(ui->chatMenu->currentItem());
            chat) {
            if (client.sendMessage(message, chat->getChatId())) {
                renderMessages(chat->getChatId());
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

void MainWindow::renderChats(const QMap<int, QSharedPointer<Chat>> &chatsList) {
    // client.getUserList(availibleUsers);
    clearListWidget(ui->chatMenu);

    for (QMap<int, QSharedPointer<Chat>>::const_iterator it =
             chatsList.constBegin();
         it != chatsList.constEnd(); ++it) {
        QListWidgetItem *item = it.value().get();
        item->setText(it.value()->getName());
        ui->chatMenu->addItem(it.value().get());
    }
}

void MainWindow::renderMessages(int chat_id) {
    if (!dynamic_cast<Chat *>(ui->chatMenu->currentItem()) ||
        dynamic_cast<Chat *>(ui->chatMenu->currentItem())->getChatId() !=
            chat_id ||
        !availableChats.contains(chat_id)) {
        return;
    }

    Chat *chat = availableChats[chat_id].get();

    client.getUserList(availibleUsers);
    QList<QSharedPointer<Message>> newHistory;
    if (client.getChatMessages(chat->getChatId(), newHistory)) {
        clearListWidget(ui->chatView);
        chat->updateMessageHistory(newHistory);

        for (QList<QSharedPointer<Message>>::iterator it =
                 chat->getHistory().begin();
             it != chat->getHistory().end(); ++it) {
            if (it->get()) {
                if (QListWidgetItem *widgetElem =
                        dynamic_cast<QListWidgetItem *>(it->get());
                    widgetElem) {
                    QString textField =
                        "[ " + availibleUsers[it->get()->getAuthorId()] +
                        " ] : " + it->get()->getText();
                    widgetElem->setText(textField);
                    ui->chatView->addItem(widgetElem);
                }
            }
            // TODO: try - catch
        }
    }
}

void MainWindow::on_chatMenu_itemClicked(QListWidgetItem *item) {
    Chat *chat = dynamic_cast<Chat *>(item);
    if (chat) {
        ui->chatView->show();
        renderMessages(chat->getChatId());
    }
}

void MainWindow::newChat(QString name) {
    if (client.createChat(name)) {
        updateChats();
    }
}

void MainWindow::updateChats() {
    QMap<int, QSharedPointer<Chat>> newAvailableChats;
    if (client.getChatList(newAvailableChats)) {
        clearListWidget(ui->chatMenu);
        availableChats.clear();
        std::swap(newAvailableChats, availableChats);
    }
    renderChats(availableChats);
}

void MainWindow::inviteUser(int id) {
    qDebug() << id;
}

void MainWindow::on_createChatButton_clicked() {
    QMap<int, QString> users;
    client.getUserList(users);
    Dialog creating(client, users);
    connect(&creating, SIGNAL(requestAddUser(int)), this,
            SLOT(inviteUser(int)));
    connect(&creating, SIGNAL(requestCreating(const QString &)), this,
            SLOT(newChat(const QString &)));

    creating.exec();
}
