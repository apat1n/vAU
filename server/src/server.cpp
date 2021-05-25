#include "server.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtCore/QDebug>
#include "QtWebSockets/qwebsocket.h"
#include "QtWebSockets/qwebsocketserver.h"

QT_USE_NAMESPACE

Server::Server(quint16 port, bool debug, QObject *parent)
    : QObject(parent),
      m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Server"),
                                              QWebSocketServer::NonSecureMode,
                                              this)),
      m_debug(debug),
      env(QProcessEnvironment::systemEnvironment()),
      db(env.value("DBURL"),
         env.value("DBPORT").toInt(),
         env.value("DBNAME"),
         env.value("DBUSERNAME"),
         env.value("DBPASSWORD"),
         m_debug) {
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "Server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
                &Server::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this,
                &Server::closed);
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
