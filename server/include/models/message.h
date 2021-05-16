#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDate>

struct Message {
    int id;
    int user_id;
    QString message;
    QDate lastUpdated;
};

#endif // MESSAGE_H
