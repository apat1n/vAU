#include "server.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcessEnvironment>
#include <QtCore/QDebug>
#include "QtWebSockets/qwebsocket.h"
#include "QtWebSockets/qwebsocketserver.h"

QT_USE_NAMESPACE

Server::Server(quint16 port, bool debug, QObject *parent)
    : QObject(parent),
      m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Server"),
                                              QWebSocketServer::NonSecureMode,
                                              this)),
      m_debug(debug) {
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "Server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
                &Server::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this,
                &Server::closed);
    }

    // make SQL connection
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    db = QSqlDatabase::addDatabase("QPSQL");

    db.setHostName(env.value("DBURL"));
    db.setPort(env.value("DBPORT").toInt());
    db.setDatabaseName(env.value("DBNAME"));
    db.setUserName(env.value("DBUSERNAME"));
    db.setPassword(env.value("DBPASSWORD"));

    if (m_debug) {
        qDebug() << "SQL connection: " << (db.open() ? "opened" : "closed");
    }

    // set English localization for dates
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

Server::~Server() {
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void Server::onNewConnection() {
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::binaryMessageReceived, this,
            &Server::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this,
            &Server::socketDisconnected);

    m_clients << pSocket;
    if (m_debug) {
        qDebug() << "newConnection:" << pSocket;
    }
}

void Server::processBinaryMessage(QByteArray message) {
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());

    QJsonObject requestObj = QJsonDocument::fromJson(message).object();
    QJsonObject requestBody = requestObj.value("request").toObject();
    QString requestMethod = requestBody.value("method").toString();

    if (m_debug) {
        qDebug() << sender() << requestObj;
    }

    if (requestMethod == "login") {
        processLoginRequest(requestBody, pSender);
    } else if (requestMethod == "register") {
        processRegisterRequest(requestBody, pSender);
    } else if (requestMethod == "getChatList") {
        processGetChatListRequest(requestBody, pSender);
    } else if (requestMethod == "logout") {
        processLogoutRequest(requestBody, pSender);
    } else if (requestMethod == "createChat") {
        processCreateChatRequest(requestBody, pSender);
    } else if (requestMethod == "sendMessage") {
        processSendMessageRequest(requestBody, pSender);
    } else if (requestMethod == "getChatMessages") {
        proccessChatGetMessages(requestBody, pSender);
    } else if (requestMethod == "getUserList") {
        processGetUserList(requestBody, pSender);
    }
}

void Server::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug) {
        qDebug() << "socketDisconnected:" << pClient;
    }
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

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
        QJsonObject response;
        response["method"] = requestBody.value("method").toString();
        response["message"] = "";
        response["status"] = 403;

        QJsonObject responseObj;
        responseObj["response"] = response;

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
        QJsonObject response;
        response["method"] = requestBody.value("method").toString();
        response["message"] = "";
        response["status"] = 403;

        QJsonObject responseObj;
        responseObj["response"] = response;

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
        QJsonObject response;
        response["method"] = requestBody.value("method").toString();
        response["message"] = "";
        response["status"] = 403;

        QJsonObject responseObj;
        responseObj["response"] = response;

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
        QJsonObject response;
        response["method"] = requestBody.value("method").toString();
        response["message"] = "";
        response["status"] = 403;

        QJsonObject responseObj;
        responseObj["response"] = response;

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
        QJsonObject response;
        response["method"] = requestBody.value("method").toString();
        response["message"] = "";
        response["status"] = 403;

        QJsonObject responseObj;
        responseObj["response"] = response;

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
