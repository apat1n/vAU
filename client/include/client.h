#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <memory>
#include <optional>
#include "chat.h"
#include "user.h"

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(const QUrl &, bool = true, QObject * = nullptr);
    void connectServer();
    void disconnectServer();
    // void sendMessage(const QString &);
    QString getState() const;

Q_SIGNALS:
    void closed();
    void responseRecieved();
    void connectionUnstable();
    void responsePushMessageReceived(int chat_id);
    void responsePushChatMessageReceived();

private Q_SLOTS:
    void onConnected();
    void onBinaryMessageReceived(QByteArray);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;

    std::optional<QJsonObject> responseObj;
    void onWebcocketStateChanged();
    int currId = 0;

public:
    int getId() const;
    bool sendMessage(const QSharedPointer<Message> &, int);
    bool logoutUser();
    bool createChat(QString name, int &chat_id);
    bool getChatList(QMap<int, QSharedPointer<Chat>> &chatList);
    bool getChatMessages(int chatId,
                         QList<QSharedPointer<Message>> &messageHistory);
    bool getUserList(QMap<int, QString> &, int chatId = -1);
    void waitResponse();
    void sendRequest(const QJsonObject &requestObj);
    bool loginUser(QString login, QString password);
    bool registerUser(QString login, QString password);
    bool updateUserPhoto(QImage &photo);
    bool getUserPhoto(QImage &photo, int userId);
    bool getContactList(QMap<int, QString> &);
    bool inviteUserChat(int user_id, int chat_id);
    bool addUserContact(int userId);
    bool getUserProfile(User &user, int user_id);
    bool updateUserStatus(QString newStatus);
    bool updateUserLogin(QString newLogin);
};

#endif  // ECHOCLIENT_H
