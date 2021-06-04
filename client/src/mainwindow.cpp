#include "mainwindow.h"
#include <QFileDialog>
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
    client.connectServer();
    QFile file(":/styles/light/style.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    ui->messageTextField->hide();
    ui->inputPassword->setEchoMode(QLineEdit::Password);
    ui->menuLog_Out->menuAction()->setVisible(false);
    ui->chatView->hide();
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget_2->setCurrentIndex(0);
    ui->stackedWidget_3->setCurrentIndex(0);
    connect(&client, &Client::responsePushMessageReceived, this,
            &MainWindow::renderMessages);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_signIn_clicked() {
    QString login = ui->inputLogin->text();
    QString password = ui->inputPassword->text();

    if (client.loginUser(login, password)) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->menuLog_Out->menuAction()->setVisible(true);
        updateChats();
        ui->errorLogin->hide();
    } else {
        ui->errorLogin->show();
    }
    ui->inputPassword->clear();
}

void MainWindow::on_registerButton_clicked() {
    QString login = ui->inputLogin->text();
    QString password = ui->inputPassword->text();

    if (client.registerUser(login, password)) {
        ui->menuLog_Out->menuAction()->setVisible(true);
        updateChats();
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
        item->setIcon(getChatImage(it.value()->getChatId()));
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
                    widgetElem->setIcon(QIcon(QPixmap::fromImage(
                        getUserImage(it->get()->getAuthorId(), client))));
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
        ui->messageTextField->show();
        renderMessages(chat->getChatId());
        ui->stackedWidget_3->setCurrentIndex(0);
    }
}

void MainWindow::newChat(QString name, const QList<int> &user_list) {
    int chat_id = 0;
    if (client.createChat(name, chat_id)) {
        for (auto it : user_list) {
            client.inviteUserChat(it, chat_id);
        }
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

void MainWindow::updateUsers() {
    QMap<int, QString> newAvailableUsers;
    if (client.getUserList(newAvailableUsers)) {
        clearListWidget(ui->friendList);
        availibleUsers.clear();
        std::swap(newAvailableUsers, availibleUsers);
    }
    QMap<int, QString> newAvailableContacts;
    if (client.getContactList(newAvailableContacts)) {
        clearListWidget(ui->friendList);
        contacts.clear();
        std::swap(newAvailableContacts, contacts);
    }
    renderUsers(contacts);
}

void MainWindow::renderUsers(const QMap<int, QString> &userList) {
    clearListWidget(ui->friendList);
    for (QMap<int, QString>::const_iterator it = userList.constBegin();
         it != userList.constEnd(); ++it) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(it.value());
        item->setData(Qt::UserRole, it.key());
        item->setIcon(
            QIcon(QPixmap::fromImage(getUserImage(it.key(), client))));
        ui->friendList->addItem(item);
    }
}

void MainWindow::inviteUser(int id) {
    qDebug() << id;
}

void MainWindow::on_actionDark_Theme_triggered() {
    if (ui->actionDark_Theme->isChecked()) {
        QFile file(":/styles/dark/style.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    } else {
        QFile file(":/styles/light/style.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    }
}

void MainWindow::on_actionLog_Out_triggered() {
    if (client.logoutUser()) {
        ui->stackedWidget->setCurrentIndex(0);
    }
    ui->menuLog_Out->menuAction()->setVisible(false);
}

void MainWindow::on_actionMy_Profile_triggered() {
    ui->stackedWidget_3->setCurrentIndex(1);
    QImage icon = getUserImage(client.getId(), client);
    ui->label->setPixmap(QPixmap::fromImage(icon));
    ui->label_2->setText(getUserStatus(client.getId()));
    QFont font = ui->label_3->font();
    font.setPointSize(24);
    ui->label_3->setFont(font);
    font = ui->label_2->font();
    font.setPointSize(12);
    ui->label_2->setFont(font);

    ui->label_4->hide();

    ui->addFriend->hide();
}

void MainWindow::on_createChatButton_clicked() {
    QMap<int, QString> users;
    client.getUserList(users);
    Dialog creating(client, users);
    connect(&creating,
            SIGNAL(requestCreating(const QString &, const QList<int> &)), this,
            SLOT(newChat(const QString &, const QList<int> &)));
    //    connect(&creating, SIGNAL(requestAddUser(int)), this,
    //            SLOT(inviteUser(int)));

    creating.exec();
}

void MainWindow::on_friends_clicked() {
    updateUsers();
    ui->stackedWidget_2->setCurrentIndex(1);
}

void MainWindow::updateUserProfile(int id, QString name) {
    QImage icon = getUserImage(id, client);
    ui->label->setPixmap(QPixmap::fromImage(icon));
    ui->label_4->show();
    QFont font = ui->label_3->font();
    font.setPointSize(24);
    ui->label_3->setFont(font);
    font = ui->label_2->font();
    font.setPointSize(12);
    ui->label_2->setFont(font);
    ui->label_2->setText(getUserStatus(id));
    ui->label_3->setText(name);
    if (contacts.contains(id)) {
        ui->label_4->setText("This user is already your friend");
        ui->addFriend->hide();
    } else {
        ui->label_4->setText("You can add this user to your friend list");
        ui->addFriend->show();
    }
}

void MainWindow::on_messages_clicked() {
    ui->stackedWidget_2->setCurrentIndex(0);
}

void MainWindow::on_friendList_itemClicked(QListWidgetItem *item) {
    updateUserProfile(item->data(Qt::UserRole).toInt(), item->text());
    ui->stackedWidget_3->setCurrentIndex(1);
}

void MainWindow::on_addFriend_clicked() {
    int userId = ui->friendList->currentItem()->data(Qt::UserRole).toInt();
    ui->label_4->setText("Your request was sent");
    ui->addFriend->hide();
    client.addUserContact(userId);
}

void MainWindow::on_actionChange_my_photo_triggered() {
    QString newPhoto = QFileDialog::getOpenFileName(
        this, "Upload a photo", "/home", "*.jpg *.png *.bmp");
    QImage icon = QImage(newPhoto).scaled(256, 256);
    ui->label->setPixmap(QPixmap::fromImage(icon));
    ui->label_2->setText(getUserStatus(client.getId()));
    QFont font = ui->label_3->font();
    font.setPointSize(24);
    ui->label_3->setFont(font);
    font = ui->label_2->font();
    font.setPointSize(12);
    ui->label_2->setFont(font);

    ui->label_4->hide();

    ui->addFriend->hide();
    ui->stackedWidget_3->setCurrentIndex(1);
    client.updateUserPhoto(icon);
}

void MainWindow::on_searchFriends_textEdited(const QString &arg1) {
    if (!arg1.isEmpty()) {
        QMap<int, QString> result;
        for (QMap<int, QString>::const_iterator it = availibleUsers.begin();
             it != availibleUsers.end(); ++it) {
            if (isMatch(it.value(), arg1)) {
                result.insert(it.key(), it.value());
            }
        }
        QMap<int, QString> tmp = result;
        renderUsers(tmp);
    } else {
        renderUsers(contacts);
    }
}
