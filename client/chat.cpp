#include "chat.h"

Chat::Chat(int chatId, const QString name_) : chatId(chatId), name(name_) {
}

[[nodiscard]] QList<Message *> Chat::getHistory() const {
    return message_history;
}

[[nodiscard]] QString Chat::getName() const {
    return name;
}

[[nodiscard]] int Chat::getChatId() const {
    return chatId;
}

void Chat::addMessage(Message *message) {
    message_history.append(message);
}
