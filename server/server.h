#ifndef SERVER_H
#define SERVER_H

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QObject>
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
    QSqlDatabase db;
    bool m_debug;

    void processLoginRequest(QJsonObject, QWebSocket *);
    void processRegisterRequest(QJsonObject, QWebSocket *);
    void processLogoutRequest(QJsonObject, QWebSocket *);
    void processGetChatListRequest(QJsonObject, QWebSocket *);
    void processCreateChatRequest(QJsonObject, QWebSocket *);
    void proccessChatGetMessages(QJsonObject, QWebSocket *);
    void processSendMessageRequest(QJsonObject, QWebSocket *);

    bool authUser(User &, QString);
    bool registerUser(QString, QString);

    bool createChat(QString, int);
    QList<Chat> getChatList(User &);
    QList<Message> getMessageList(User &, int chatId);

    bool createMessage(int, int, QString, QDate);
};

#endif  // SERVER_H
