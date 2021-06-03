#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <chat.h>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <memory>
#include <optional>

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
    bool createChat(QString name);
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
    bool addUserContact(int userId);
};

#endif  // ECHOCLIENT_H
