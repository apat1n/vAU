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

bool Server::registerUser(QString login, QString password) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "INSERT INTO QUser (login, password_hash) VALUES ('"
       << login.toStdString() << "', '" << password.toStdString() << "');";

    return query.exec(QString::fromStdString(ss.str()));
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

QList<Chat> Server::getChatList(User &user) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT id, name FROM QChat JOIN QChatUserList ON chat_id = id WHERE "
          "user_id = "
       << user.id << ";";
    qDebug() << QString::fromStdString(ss.str());
    query.exec(QString::fromStdString(ss.str()));

    QList<Chat> result;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        //        QDate lastUpdated = query.value(2).toDate();
        result.append(Chat{id, name, QDate()});
    }
    return result;
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

QList<Message> Server::getMessageList(int chatId) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT id, user_id, message, message_date FROM QChatMessages WHERE "
          "chat_id = "
       << chatId << ";";

    qDebug() << QString::fromStdString(ss.str());
    query.exec(QString::fromStdString(ss.str()));

    QList<Message> result;
    while (query.next()) {
        int id = query.value(0).toInt();
        int user_id = query.value(1).toInt();
        QString message = query.value(2).toString();
        //        QDate lastUpdated = query.value(3).toDate();
        result.append(Message{id, user_id, message, QDate()});
    }
    return result;
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

bool Server::createChat(QString name, int user_id) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "INSERT INTO QChat (name, creator_id) VALUES ('" << name.toStdString()
       << "', " << user_id << ");";
    qDebug() << QString::fromStdString(ss.str());

    if (query.exec(QString::fromStdString(ss.str()))) {
        int chat_id = query.lastInsertId().toInt();
        ss.str("");  // clear query string
        ss << "INSERT INTO QChatUserList (chat_id, user_id) VALUES (" << chat_id
           << ", " << user_id << ");";
        qDebug() << QString::fromStdString(ss.str());
        return query.exec(QString::fromStdString(ss.str()));
    }
    return false;
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

bool Server::createMessage(int chat_id,
                           int user_id,
                           QString message,
                           QDate date) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "INSERT INTO QChatMessages (chat_id, user_id, message, message_date) "
       << "VALUES (" << chat_id << ", " << user_id << ", '"
       << message.toStdString() << "', '"
       << date.toString(Qt::DefaultLocaleShortDate).toStdString() << "');";
    qDebug() << QString::fromStdString(ss.str());
    return query.exec(QString::fromStdString(ss.str()));
}
