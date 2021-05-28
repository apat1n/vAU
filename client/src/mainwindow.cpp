#include "mainwindow.h"
#include <iostream>
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
    QFile file("/home/vtgcon/Загрузки/style0.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    ui->messageTextField->hide();
    ui->inputPassword->setEchoMode(QLineEdit::Password);
    ui->stackedWidget->setCurrentIndex(0);
    ui->menuLog_Out->menuAction()->setVisible(false);
    ui->chatView->hide();
    ui->stackedWidget_3->setCurrentIndex(0);
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
        Message *message = new Message(messageText, client.getId());
        message->setText(messageText);
        //

        // Sending to db and adding to listview
        if (Chat *chat = dynamic_cast<Chat *>(ui->chatMenu->currentItem());
            chat) {
            if (client.sendMessage(message, chat->getChatId())) {
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
    for (auto it : chatsList) {
        QListWidgetItem *item = it;
        item->setIcon(getChatImage(it->getChatId()));
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
                if (QListWidgetItem *widgetElem =
                        dynamic_cast<QListWidgetItem *>(it);
                    widgetElem) {
                    QString textField = "[ " +
                                        availibleUsers[it->getAuthorId()] +
                                        " ] : " + it->getText();
                    qDebug() << textField;
                    widgetElem->setText(textField);
                    widgetElem->setIcon(getUserImage(it->getAuthorId()));
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
        renderMessages(chat);
        ui->stackedWidget_3->setCurrentIndex(0);
    }
}

void MainWindow::newChat(QString name) {
    if (client.createChat(name)) {
        updateChats();
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

void MainWindow::updateUsers() {
    QMap<int, QString> newUsers;
    if (client.getUserList(newUsers)) {
        clearListWidget(ui->friendList);
        QMap<int, QString>::Iterator i;
        for(i = newUsers.begin(); i != newUsers.end(); i++) {
//            qDebug() << i.key() << " ";
            QListWidgetItem *it = new QListWidgetItem;
            it->setText(i.value());
            it->setIcon(getUserImage(i.key()));
            it->setData(Qt::UserRole, i.key());
            ui->friendList->addItem(it);
        }
    }
}

void MainWindow::inviteUser(int id) {
    qDebug() << id;
}

void MainWindow::on_actionDark_Theme_triggered() {
    if (ui->actionDark_Theme->isChecked()) {
        QFile file("/home/vtgcon/Загрузки/style.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    } else {
        QFile file("/home/vtgcon/Загрузки/style0.qss");
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
    QIcon icon = getUserImage(client.getId());
    ui->label->setPixmap(icon.pixmap(icon.actualSize(QSize(256,256))));
    ui->label_2->setText(getUserStatus(client.getId()));
}

void MainWindow::on_createChatButton_clicked() {
    QMap<int, QString> users;
    client.getUserList(users);
    Dialog creating(users);

    connect(&creating, SIGNAL(requestAddUser(int)), this,
            SLOT(inviteUser(int)));
    connect(&creating, SIGNAL(requestCreating(const QString &)), this,
            SLOT(newChat(const QString &)));

    creating.exec();
}

void MainWindow::on_friends_clicked()
{
    updateUsers();
    ui->stackedWidget_2->setCurrentIndex(1);
}

void MainWindow::updateUserProfile(int id){
    QIcon icon = getUserImage(id);
    ui->label->setPixmap(icon.pixmap(icon.actualSize(QSize(256,256))));
    ui->label_2->setText(getUserStatus(id));
}

void MainWindow::on_messages_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(0);
}

void MainWindow::on_friendList_itemDoubleClicked(QListWidgetItem *item)
{
    updateUserProfile(item->data(Qt::UserRole).toInt());
    ui->stackedWidget_3->setCurrentIndex(1);
}
