#ifndef CHAT_H
#define CHAT_H
#include <QList>
#include <QString>
#include "common.h"
#include "user.h"

struct Chat {
    QString chatId;

    QList<User *> getUsers();
};

#endif  // CHAT_H
