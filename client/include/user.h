#ifndef USER_H
#define USER_H
#include <QList>
#include <QString>
#include <QWebSocket>

struct User {
    int id;
    QString login;
    QString status;
    QWebSocket *senderId;
};

#endif  // USER_H
