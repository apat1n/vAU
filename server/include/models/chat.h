#ifndef CHAT_H
#define CHAT_H
#include <QList>
#include <QString>
#include "common.h"
#include "user.h"

struct Chat {
    int id;
    QString name;
    bool is_group;
    QDate lastUpdated;
};

#endif  // CHAT_H
