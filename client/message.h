#ifndef MESSAGE_H
#define MESSAGE_H

#include <QListWidgetItem>

class Message: public QListWidgetItem {
private:
    QString textMessage;
    QString author;

public:
    Message(const QString&, const QString&);
    [[nodiscard]] QString getText() const;
    [[nodiscard]] QString getAuthor() const;
};

#endif // MESSAGE_H
