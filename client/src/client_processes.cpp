#include <QJsonArray>
#include "client.h"
#include "utils.cpp"

bool Client::sendMessage(const QSharedPointer<Message> &messageObj,
                         int chatId) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["content"] = messageObj->getText();
    message["chatId"] = chatId;

    QString sentMethod = "sendMessage";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::createChat(QString name, int &chat_id) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["name"] = name;

    QString sentMethod = "createChat";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));
    sendRequest(requestObj);
    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    chat_id = responseBody.value("message")
                  .toObject()
                  .value("content")
                  .toObject()
                  .value("chat_id")
                  .toInt();

    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::getChatList(QMap<int, QSharedPointer<Chat>> &chatList) {
    Q_ASSERT(!responseObj.has_value());
    QString sentMethod = "getChatList";
    QJsonObject requestObj = getJsonRequestInstance(sentMethod);
    sendRequest(requestObj);

    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    if (method == sentMethod && status == 200) {
        QJsonArray responseArray =
            responseBody.value("message").toObject().value("content").toArray();
        for (auto chatData : responseArray) {
            int id = chatData.toObject().value("id").toInt();
            QString name = chatData.toObject().value("name").toString();
            chatList.insert(id, QSharedPointer<Chat>(new Chat(id, name)));
        }
    }

    return method == sentMethod && status == 200;
}

bool Client::getChatMessages(int chatId,
                             QList<QSharedPointer<Message>> &messageHistory) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["chat_id"] = chatId;

    QString sentMethod = "getChatMessages";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    if (method == sentMethod && status == 200) {
        QJsonArray responseArray =
            responseBody.value("message").toObject().value("content").toArray();
        for (auto responseMessage : responseArray) {
            int id = responseMessage.toObject().value("id").toInt();
            QString messageText =
                responseMessage.toObject().value("message_text").toString();
            messageHistory.append(
                QSharedPointer<Message>(new Message(messageText, id)));
        }
    }

    return method == sentMethod && status == 200;
}

bool Client::logoutUser() {
    Q_ASSERT(!responseObj.has_value());
    QString sentMethod = "logout";
    QJsonObject requestObj = getJsonRequestInstance(sentMethod);

    sendRequest(requestObj);
    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::loginUser(QString login, QString password) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["login"] = login;
    message["password"] =
        QString(QCryptographicHash::hash(password.toUtf8(),
                                         QCryptographicHash::RealSha3_256)
                    .toBase64());

    QString sentMethod = "login";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QJsonObject responseMessage = responseBody.value("message").toObject();
    QJsonObject content = responseMessage.value("content").toObject();
    currId = content["id"].toInt();

    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    responseObj.reset();
    return method == sentMethod && status == 200;
}

bool Client::registerUser(QString login, QString password) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["login"] = login;
    message["password"] =
        QString(QCryptographicHash::hash(password.toUtf8(),
                                         QCryptographicHash::RealSha3_256)
                    .toBase64());

    QString sentMethod = "register";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QJsonObject responseMessage = responseBody.value("message").toObject();
    QJsonObject content = responseMessage.value("content").toObject();
    currId = content["id"].toInt();

    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::getUserList(QMap<int, QString> &userList, int chatId) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["chatId"] = chatId;
    QString sentMethod = "getUserList";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);

    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();

    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    QJsonArray responseArray =
        responseBody.value("message").toObject().value("content").toArray();
    responseObj.reset();
    for (auto user_it : responseArray) {
        int user_id = user_it.toObject().value("user_id").toInt();
        QString user_name = user_it.toObject().value("user_name").toString();

        userList[user_id] = user_name;
    }

    return method == sentMethod && status == 200;
}

bool Client::getContactList(QMap<int, QString> &contactList) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    QString sentMethod = "getContactList";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);

    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();

    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    QJsonArray responseArray =
        responseBody.value("message").toObject().value("content").toArray();
    responseObj.reset();
    for (auto user_it : responseArray) {
        int user_id = user_it.toObject().value("user_id").toInt();
        QString user_name = user_it.toObject().value("user_name").toString();
        contactList[user_id] = user_name;
    }

    return method == sentMethod && status == 200;
}

bool Client::addUserContact(int userId) {
    Q_ASSERT(!responseObj.has_value());
    QJsonObject message;
    message["contactUserId"] = userId;

    QString sentMethod = "addUserContact";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);

    if (!responseObj.has_value()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::updateUserPhoto(QImage &photo) {
    QJsonObject message;
    message["content"] = imageToBase64(photo);

    QString sentMethod = "updateUserPhoto";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::getUserPhoto(QImage &photo, int userId = -1) {
    qDebug() << "here";
    QJsonObject message;
    message["userId"] = userId;

    QString sentMethod = "getUserPhoto";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    QJsonObject responseMessage = responseBody.value("message").toObject();
    QJsonObject content = responseMessage.value("content").toObject();
    QString photoBase64 = content.value("image").toString();

    photo =
        QImage::fromData(QByteArray::fromBase64(photoBase64.toUtf8()), "png");
    if (m_debug) {
        qDebug() << "Get image with size" << photo.size();
    }

    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::inviteUserChat(int user_id, int chat_id) {
    QJsonObject message;
    message["userId"] = user_id;
    message["chatId"] = chat_id;

    QString sentMethod = "inviteUserChat";

    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::getUserProfile(User &user, int user_id) {
    QJsonObject message;
    message["userId"] = user_id;

    QString sentMethod = "getUserProfile";

    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QJsonObject responseMessage = responseBody.value("message").toObject();
    QJsonObject content = responseMessage.value("content").toObject();
    QString method = responseBody.value("method").toString();

    user.id = content["id"].toInt();
    user.login = content["login"].toString();
    user.status = content["status"].toString();

    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::updateUserStatus(QString newStatus) {
    QJsonObject message;
    message["status"] = newStatus;

    QString sentMethod = "updateUserStatus";

    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}

bool Client::updateUserLogin(QString newLogin) {
    QJsonObject message;
    message["login"] = newLogin;

    QString sentMethod = "updateUserLogin";

    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMethod && status == 200;
}
