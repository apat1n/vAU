#ifndef SERVER_H
#define SERVER_H

#include <QProcessEnvironment>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QObject>
#include "database.h"
#include "models/chat.h"
#include "models/message.h"
#include "models/user.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(quint16 port,
                    bool debug = false,
                    QObject *parent = nullptr);
    ~Server();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QMap<QWebSocket *, User> authenticatedUsers;
    QMap<int, QWebSocket *> authenticatedUsersId;

    bool m_debug;

    QProcessEnvironment env;
    Database db;

    void processLoginRequest(QJsonObject, QWebSocket *);
    void processRegisterRequest(QJsonObject, QWebSocket *);
    void processLogoutRequest(QJsonObject, QWebSocket *);
    void processGetChatListRequest(QJsonObject, QWebSocket *);
    void processCreateChatRequest(QJsonObject, QWebSocket *);
    void proccessChatGetMessages(QJsonObject, QWebSocket *);
    void processSendMessageRequest(QJsonObject, QWebSocket *);
    void processGetUserList(QJsonObject, QWebSocket *);
    void processUpdateUserPhoto(QJsonObject, QWebSocket *);
    void processGetUserPhoto(QJsonObject, QWebSocket *);
    void processGetContactList(QJsonObject, QWebSocket *);
    void processAddUserContact(QJsonObject, QWebSocket *);
    void processInviteUserChat(QJsonObject, QWebSocket *);
    void processGetUserProfile(QJsonObject, QWebSocket *);
    void processUpdateUserStatus(QJsonObject, QWebSocket *);
    void processUpdateUserLogin(QJsonObject, QWebSocket *);

    bool isAuthorized(const QJsonObject &, QWebSocket *);
    bool updateUserPhoto(User &, QImage &);
};

#endif  // SERVER_H
