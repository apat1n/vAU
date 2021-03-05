#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDate>

struct Message {
    int id;
    QString message;
    QDate lastUpdated;
};

#endif // MESSAGE_H
