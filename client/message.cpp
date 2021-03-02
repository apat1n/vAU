#include "message.h"

Message::Message(const QString &textMessage, const QString &author): textMessage(textMessage), author(author) {
}

QString Message::getText() const {
    return textMessage;
}

QString Message::getAuthor() const {
    return author;
}