#include "server.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcessEnvironment>
#include <QtCore/QDebug>
#include <sstream>
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
        //        processSendMessageRequest(requestBody, pSender);
        //        for (QWebSocket *pClient : qAsConst(m_clients)) {
        //            if (pClient != pSender) {
        //                pClient->sendTextMessage(message);
        //            }
        //        }
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

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

bool Server::authUser(User &user, QString password) {
    QSqlQuery query(db);
    QString queryLogin = user.login;
    std::stringstream ss;
    ss << "SELECT * FROM QUser WHERE login = '" << queryLogin.toStdString()
       << "';";
    query.exec(QString::fromStdString(ss.str()));

    if (query.size() == 0) {
        return false;
    }

    query.next();
    int id = query.value(0).toInt();
    QString login = query.value(1).toString();
    QString password_hash = query.value(2).toString();

    user.id = id;

    return password_hash == password;
}

std::optional<User> Server::processRegisterRequest(QJsonObject requestBody,
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

    std::optional<User> user;
    if (response["status"] == 200) {
        user = User({-1, login, pSender});
        authUser(*user, password);
    }
    return user;
}

bool Server::registerUser(QString login, QString password) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "INSERT INTO QUser (login, password_hash) VALUES ('"
       << login.toStdString() << "', '" << password.toStdString() << "');";

    return query.exec(QString::fromStdString(ss.str()));
}

void Server::processLogoutRequest(QJsonObject requestBody, QWebSocket *pSender) {
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
    response["message"] = "";
    response["content"] = QJsonArray();

    User user = authenticatedUsers[pSender];
    for (auto chat : getChatList(user)) {
    }

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

QList<Chat> Server::getChatList(User &user) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT id, name FROM QChat JOIN QChatUserList ON chat_id = id WHERE "
          "user_id = "
       << user.id << ";";

    QList<Chat> result;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        QDate lastUpdated = query.value(2).toDate();
        result << Chat{id, name, lastUpdated};
    }
    return result;
}

void Server::processCreateChatRequest(QJsonObject requestBody, QWebSocket *pSender) {
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
    response["content"] = QJsonArray();

    User user = authenticatedUsers[pSender];
    for (auto chat : getChatList(user)) {
    }

    QJsonObject responseObj;
    responseObj["response"] = response;

    QByteArray responseBinaryMessage = QJsonDocument(responseObj).toJson();
    if (m_debug) {
        qDebug() << "response" << responseBinaryMessage;
    }
    pSender->sendBinaryMessage(responseBinaryMessage);
}

bool Server::createChat(QString name) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "INSERT INTO QChat (name) VALUES ('" << name.toStdString() << "');";

    return query.exec(QString::fromStdString(ss.str()));
}
