#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcessEnvironment>
#include <QtCore/QDebug>
#include "QtWebSockets/qwebsocket.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "server.h"
#include "utils.cpp"

void Server::processLoginRequest(QJsonObject requestBody, QWebSocket *pSender) {
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString login = requestMessage.value("login").toString();
    QString password = requestMessage.value("password").toString();

    User user{-1, login, pSender};

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["message"] = "";

    if (authUser(user, password)) {
        authenticatedUsers[pSender] = user;
        response["status"] = 200;
    } else {
        response["status"] = 401;
    }

    response["id"] = user.id;

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processRegisterRequest(QJsonObject requestBody,
                                    QWebSocket *pSender) {
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString login = requestMessage.value("login").toString();
    QString password = requestMessage.value("password").toString();

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["message"] = "";

    if (registerUser(login, password)) {
        response["status"] = 200;
    } else {
        response["status"] = 409;
    }

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }

    pSender->sendBinaryMessage(responseBinaryMessage);

    if (response["status"] == 200) {
        User user = User({-1, login, pSender});
        authUser(user, password);
        response["id"] = user.id;
        authenticatedUsers[pSender] = user;
    }
}

void Server::processLogoutRequest(QJsonObject requestBody,
                                  QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        authenticatedUsers.remove(pSender);
    }

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["message"] = "";
    response["status"] = 200;

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processGetChatListRequest(QJsonObject requestBody,
                                       QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        QJsonObject responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 403);
        QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
        return;
    }

    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString login = requestMessage.value("login").toString();
    QString password = requestMessage.value("password").toString();
    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["status"] = 200;

    QJsonObject responseMessage;

    QJsonArray contentArray;
    User user = authenticatedUsers[pSender];
    for (auto chat : getChatList(user)) {
        QJsonObject chatItem;
        chatItem["id"] = chat.id;
        chatItem["name"] = chat.name;
        //        chatItem["lastUpdated"] = chat.lastUpdated.toString();
        contentArray.append(chatItem);
    }

    responseMessage["content"] = contentArray;
    response["message"] = responseMessage;

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::proccessChatGetMessages(QJsonObject requestBody,
                                     QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        QJsonObject responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 403);
        QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
        return;
    }
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString login = requestMessage.value("login").toString();
    QString password = requestMessage.value("password").toString();

    int chatId =
        requestMessage.value("content").toObject().value("chat_id").toInt();

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["status"] = 200;

    QJsonObject responseMessage;

    QJsonArray contentArray;

    for (auto message_it : getMessageList(chatId)) {
        QJsonObject chatItem;
        chatItem["id"] = message_it.user_id;
        chatItem["message_text"] = message_it.message;
        //        chatItem["lastUpdated"] = chat.lastUpdated.toString();
        contentArray.append(chatItem);
    }

    responseMessage["content"] = contentArray;
    response["message"] = responseMessage;

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processCreateChatRequest(QJsonObject requestBody,
                                      QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        QJsonObject responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 403);
        QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
        return;
    }

    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString name = requestMessage.value("name").toString();

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["message"] = "";

    int user_id = authenticatedUsers[pSender].id;
    if (createChat(name, user_id)) {
        response["status"] = 200;
    } else {
        response["status"] = 401;
    }

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processSendMessageRequest(QJsonObject requestBody,
                                       QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        QJsonObject responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 403);
        QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
        return;
    }
    // if user in chat

    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString name = requestMessage.value("name").toString();

    int chat_id = requestMessage.value("chatId").toInt();
    int user_id = authenticatedUsers[pSender].id;
    QString text_message = requestMessage.value("content").toString();
    QDate date = QDate::currentDate();

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["message"] = "";

    if (createMessage(chat_id, user_id, text_message, date)) {
        response["status"] = 200;
    } else {
        response["status"] = 401;
    }

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processGetUserList(QJsonObject requestBody, QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        QJsonObject responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 403);
        QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
        return;
    }
    // if user in chat

    QJsonArray contentArray;

    QMap<int, QString> userList = getUserList();
    QMap<int, QString>::iterator user_it;
    for (user_it = userList.begin(); user_it != userList.end(); ++user_it) {
        QJsonObject chatItem;
        chatItem["user_id"] = user_it.key();
        chatItem["user_name"] = user_it.value();
        if (m_debug) {
            qDebug() << user_it.key() << " " << user_it.value();
        }
        contentArray.append(chatItem);
    }
    QJsonObject responseMessage;
    responseMessage["content"] = contentArray;

    QJsonObject response;
    response["method"] = requestBody.value("method").toString();
    response["message"] = responseMessage;
    response["status"] = 200;

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}
