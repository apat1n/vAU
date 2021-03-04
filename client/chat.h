#ifndef CHAT_H
#define CHAT_H

#include <QListWidgetItem>
#include "message.h"

class Chat : public QListWidgetItem {
private:
    int chatId = 0;
    QString name;
    QList<QString> users;
    QList<Message *> message_history;

public:
    Chat(int, const QString);
    [[nodiscard]] QList<Message *> getHistory() const;
    [[nodiscard]] QString getName() const;
    [[nodiscard]] int getChatId() const;
    void addMessage(Message *);
    // void updateHistory();
};

#endif  // CHAT_H
