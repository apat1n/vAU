#include "chat.h"

Chat::Chat(int chatId, const QString& interlocutor): chatId(chatId), interlocutor(interlocutor){}

QList<Message*> Chat::getHistory() const {
    return message_history;
}

QString Chat::getInterlocutor() const {
    return interlocutor;
}

void Chat::addMessage(Message* message) {
    message_history.append(message);
}
