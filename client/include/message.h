#ifndef MESSAGE_H
#define MESSAGE_H

#include <QListWidgetItem>
#include <QString>

class Message : public QListWidgetItem {
private:
    QString textMessage;
    QString author;
    int authorId;

public:
    Message(const QString &, const QString &);
    Message(const QString &, int autorId);

    [[nodiscard]] int getAuthorId() const;
    [[nodiscard]] QString getText() const;
    [[nodiscard]] QString getAuthor() const;
};

#endif  // MESSAGE_H
