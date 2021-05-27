#include "chat.h"

Chat::Chat(int chatId, const QString name_) : chatId(chatId), name(name_) {
}

[[nodiscard]] QList<QSharedPointer<Message>> &Chat::getHistory() {
    return message_history;
}

[[nodiscard]] QString Chat::getName() const {
    return name;
}

[[nodiscard]] int Chat::getChatId() const {
    return chatId;
}

void Chat::addMessage(QSharedPointer<Message> message) {
    message_history.append(message);
}

void Chat::updateMessageHistory(QList<QSharedPointer<Message>> &newHistory) {
    message_history.clear();
    std::swap(newHistory, message_history);
}
