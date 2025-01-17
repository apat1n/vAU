#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QtCore/QObject>
#include "models/chat.h"
#include "models/message.h"
#include "models/user.h"

class Database : public QObject {
    bool m_debug;
    QSqlDatabase db;

public:
    explicit Database(const QString &url,
                      int port,
                      const QString &name,
                      const QString &username,
                      const QString &password,
                      bool debug = false);

    bool authUser(User &user, QString password);
    bool registerUser(QString login, QString password);

    bool createChat(QString name, int user_id, int &chat_id);
    bool createMessage(int chat_id, int user_id, QString message, QDate date);
    QList<Chat> getChatList(User &user);
    QList<Message> getMessageList(int chatId);
    QMap<int, QString> getUserList(int chatId);
    QList<User> getUserContacts(int user_id);
    bool addUserContact(int user_id1, int user_id2);
    bool inviteUserChat(int user_id, int chat_id);
    User getUserProfile(int user_id);
    bool updateUserStatus(int user_id, QString status);
    bool updateUserLogin(int user_id, QString login);
};

#endif  // DATABASE_H
