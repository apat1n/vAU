#include <QJsonArray>
#include "client.h"
#include "utils.cpp"

bool Client::sendMessage(Message *messageObj, int chatId) {
    QJsonObject message;
    message["content"] = messageObj->getText();
    message["chatId"] = chatId;

    QString sentMessage = "sendMessage";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMessage, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMessage && status == 200;
}

bool Client::createChat(QString name) {
    QJsonObject message;
    message["name"] = name;

    QString sentMessage = "createChat";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMessage, std::move(message));
    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == sentMessage && status == 200;
}

bool Client::getChatList(QList<Chat *> &chatList) {
    QString sentMethod = "getChatList";
    QJsonObject requestObj = getJsonRequestInstance(sentMethod);
    sendRequest(requestObj);

    if (responseObj->empty()) {
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
            chatList.append(new Chat(id, name));
        }
    }

    return method == sentMethod && status == 200;
}

bool Client::getChatMessages(int chatId, QList<Message *> &messageHistory) {
    QJsonObject message;
    message["chat_id"] = chatId;

    QString sentMethod = "getChatMessages";
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

    if (method == sentMethod && status == 200) {
        QJsonArray responseArray =
            responseBody.value("message").toObject().value("content").toArray();
        for (auto responseMessage : responseArray) {
            int id = responseMessage.toObject().value("id").toInt();
            QString messageText =
                responseMessage.toObject().value("message_text").toString();
            messageHistory.append(new Message(messageText, id));
        }
    }

    return method == sentMethod && status == 200;
}

bool Client::logoutUser() {
    QString sentMethod = "logout";
    QJsonObject requestObj = getJsonRequestInstance(sentMethod);

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

bool Client::loginUser(QString login, QString password) {
    QJsonObject message;
    message["login"] = login;
    message["password"] = password;

    QString sentMessage = "login";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMessage, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    currId = responseBody.value("id").toInt();

    responseObj.reset();
    return method == sentMessage && status == 200;
}

bool Client::registerUser(QString login, QString password) {
    QJsonObject message;
    message["login"] = login;
    message["password"] = password;

    QString sentMessage = "register";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMessage, std::move(message));

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    currId = responseBody.value("id").toInt();

    responseObj.reset();

    return method == sentMessage && status == 200;
}

bool Client::getUserList(QMap<int, QString> &userList, int chatId) {
    QJsonObject message;
    message["chatId"] = chatId;
    QString sentMethod = "getUserList";
    QJsonObject requestObj =
        getJsonRequestInstance(sentMethod, std::move(message));

    sendRequest(requestObj);

    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();

    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();

    QJsonArray responseArray =
        responseBody.value("message").toObject().value("content").toArray();
    for (auto user_it : responseArray) {
        int user_id = user_it.toObject().value("user_id").toInt();
        QString user_name = user_it.toObject().value("user_name").toString();

        if (m_debug) {
            qDebug() << user_id << " " << user_name;
        }

        userList[user_id] = user_name;
    }

    responseObj.reset();

    return method == sentMethod && status == 200;
}
