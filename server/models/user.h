#ifndef USER_H
#define USER_H
#include <QList>
#include <QString>
#include <QWebSocket>
#include "chat.h"
#include "common.h"

struct User {
    QString login;
    QWebSocket *senderId;
    bool isAuthenticated = false;

    QList<Chat *> getChats();
};

#endif  // USER_H
