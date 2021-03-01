#ifndef USER_H
#define USER_H
#include <QList>
#include <QString>
#include <QWebSocket>
#include "chat.h"
#include "common.h"

struct User {
    int id;
    QString login;
    QWebSocket *senderId;
};

#endif  // USER_H
