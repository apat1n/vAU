#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPixmap>
#include <QProcessEnvironment>
#include <QtCore/QDebug>
#include "QtWebSockets/qwebsocket.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "server.h"
#include "utils.cpp"

bool Server::isAuthorized(const QJsonObject &requestBody, QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        QJsonObject responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 403);
        QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
        pSender->sendBinaryMessage(responseBinaryMessage);
        return false;
    }
    return true;
}

void Server::processLoginRequest(QJsonObject requestBody, QWebSocket *pSender) {
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString login = requestMessage.value("login").toString();
    QString password = requestMessage.value("password").toString();

    User user{-1, login, pSender};

    QJsonObject responseObj;
    if (db.authUser(user, password)) {
        authenticatedUsers[pSender] = user;
        authenticatedUsersId[user.id] = pSender;

        QJsonObject content;
        content["id"] = user.id;
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), std::move(content), 200);
    } else {
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 401);
    }

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug("processLoginRequest response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processRegisterRequest(QJsonObject requestBody,
                                    QWebSocket *pSender) {
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString login = requestMessage.value("login").toString();
    QString password = requestMessage.value("password").toString();

    QJsonObject responseObj;
    if (db.registerUser(login, password)) {
        User user = User({-1, login, pSender});
        db.authUser(user, password);
        authenticatedUsers[pSender] = user;
        authenticatedUsersId[user.id] = pSender;

        QJsonObject content;
        content["id"] =
            user.id;  // TODO: creating new ID when registering new user
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), std::move(content), 200);
    } else {
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 409);
    }
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug("processRegisterRequest response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processLogoutRequest(QJsonObject requestBody,
                                  QWebSocket *pSender) {
    if (!authenticatedUsers.contains(pSender)) {
        authenticatedUsersId.remove(authenticatedUsers[pSender].id);
        authenticatedUsers.remove(pSender);
    }

    QJsonObject responseObj =
        getJsonResponseInstance(requestBody.value("method").toString(), 200);
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug("processLogout response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processGetChatListRequest(QJsonObject requestBody,
                                       QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }

    QJsonArray contentArray;
    /* Filling contentArray */ {
        User user = authenticatedUsers[pSender];
        for (auto chat : db.getChatList(user)) {
            QJsonObject chatItem;
            chatItem["id"] = chat.id;
            chatItem["name"] = chat.name;
            //        chatItem["lastUpdated"] = chat.lastUpdated.toString();
            contentArray.append(chatItem);
        }
    }

    QJsonObject responseObj = getJsonResponseInstance(
        requestBody.value("method").toString(), std::move(contentArray), 200);
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug("processGetChatList response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::proccessChatGetMessages(QJsonObject requestBody,
                                     QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }

    QJsonArray contentArray;
    /* Filling contentArray */ {
        int chatId =
            requestBody.value("message").toObject().value("chat_id").toInt();
        for (auto message_it : db.getMessageList(chatId)) {
            QJsonObject chatItem;
            chatItem["id"] = message_it.user_id;
            chatItem["message_text"] = message_it.message;
            //        chatItem["lastUpdated"] = chat.lastUpdated.toString();
            contentArray.append(chatItem);
        }
    }

    QJsonObject responseObj = getJsonResponseInstance(
        requestBody.value("method").toString(), std::move(contentArray), 200);
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug("processGetChatMessages response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processCreateChatRequest(QJsonObject requestBody,
                                      QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }

    // Arguments for createChat() function
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString name = requestMessage.value("name").toString();
    int user_id = authenticatedUsers[pSender].id;

    QJsonObject responseObj;
    if (db.createChat(name, user_id)) {
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 200);
    } else {
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 401);
    }
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug("processCreateCharRequest response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processSendMessageRequest(QJsonObject requestBody,
                                       QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }
    // if user in chat

    // arguments for createMessage() function
    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString name = requestMessage.value("name").toString();
    int chat_id = requestMessage.value("chatId").toInt();
    int user_id = authenticatedUsers[pSender].id;
    QString text_message = requestMessage.value("content").toString();
    QDate date = QDate::currentDate();

    QJsonObject responseObj, responsePushObj;

    if (db.createMessage(chat_id, user_id, text_message, date)) {
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 200);

        QJsonObject message;
        message["push"] = 1;
        message["chat_id"] = chat_id;

        responsePushObj = getJsonResponseInstance(
            requestBody.value("method").toString(), std::move(message), 200);

    } else {
        responseObj = getJsonResponseInstance(
            requestBody.value("method").toString(), 401);
    }

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    QByteArray responsePushBinaryMessage =
        QJsonDocument(responsePushObj).toJson();

    QMap<int, QString> userList = db.getUserList(chat_id);
    for (QMap<int, QString>::iterator user_it = userList.begin();
         user_it != userList.end(); ++user_it) {
        if (authenticatedUsersId.contains(user_it.key())) {
            if (authenticatedUsersId[user_it.key()] == pSender) {
                authenticatedUsersId[user_it.key()]->sendBinaryMessage(
                    responseBinaryMessage);
                qDebug("processSendMessageRequest sent to sender %s: %s\n",
                       qUtf8Printable(user_it.value()),
                       qUtf8Printable(responseBinaryMessage));
            } else {
                authenticatedUsersId[user_it.key()]->sendBinaryMessage(
                    responsePushBinaryMessage);
                qDebug("processSendMessageRequest sent push to %s: %s\n",
                       qUtf8Printable(user_it.value()),
                       qUtf8Printable(responsePushBinaryMessage));
            }
        }
    }
}

void Server::processGetUserList(QJsonObject requestBody, QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }
    // if user in chat

    int chatId =
        requestBody.value("message").toObject().value("chatId").toInt();

    QJsonArray contentArray;
    /* Filling contentArray */ {
        QMap<int, QString> userList = db.getUserList(chatId);
        QMap<int, QString>::iterator user_it;
        for (user_it = userList.begin(); user_it != userList.end(); ++user_it) {
            QJsonObject chatItem;
            chatItem["user_id"] = user_it.key();
            chatItem["user_name"] = user_it.value();
            contentArray.append(chatItem);
        }
    }

    QJsonObject responseObj = getJsonResponseInstance(
        requestBody.value("method").toString(), std::move(contentArray), 200);
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug("processGetUserList response: %s\n",
               qUtf8Printable(responseBinaryMessage));
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processUpdateUserPhoto(QJsonObject requestBody,
                                    QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }

    QJsonObject requestMessage = requestBody.value("message").toObject();
    QString photoBase64 = requestMessage.value("content").toString();
    int userId = authenticatedUsers[pSender].id;

    QImage image =
        QImage::fromData(QByteArray::fromBase64(photoBase64.toUtf8()), "png");
    if (m_debug) {
        qDebug() << "Get image with size" << image.size();
    }

    // TODO: сделать запись в БД
    saveImage(image, QString("user_original_%1").arg(userId));

    QJsonObject responseObj =
        getJsonResponseInstance(requestBody.value("method").toString(), 200);
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

void Server::processGetUserPhoto(QJsonObject requestBody, QWebSocket *pSender) {
    if (!isAuthorized(requestBody, pSender)) {
        return;
    }

    int userId = authenticatedUsers[pSender].id;

    // TODO: читать путь из БД
    QImage photo;
    try {
        photo = loadImage(QString("user_original_%1").arg(userId));
    } catch (...) {
        photo = QImage(256, 256, QImage::Format_RGB32);
        photo.fill(Qt::blue);
    }

    if (m_debug) {
        qDebug() << "Send image with size" << photo.size();
    }

    QJsonObject message;
    message["image"] = imageToBase64(photo);

    QString sentMethod = "updateUserPhoto";
    QJsonObject responseObj = getJsonResponseInstance(
        requestBody.value("method").toString(), std::move(message), 200);
    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();

    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}
