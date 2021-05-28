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

    bool createChat(QString name, int user_id);
    bool createMessage(int chat_id, int user_id, QString message, QDate date);
    QList<Chat> getChatList(User &user);
    QList<Message> getMessageList(int chatId);
    QMap<int, QString> getUserList(int chatId);
};

#endif  // DATABASE_H
