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
#include <optional>

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(const QUrl &, bool = true, QObject * = nullptr);
    void connectServer();
    void disconnectServer();
    void sendMessage(const QString &);
    QString getState() const;

Q_SIGNALS:
    void closed();
    void responseRecieved();
    void connectionUnstable();

private Q_SLOTS:
    void onConnected();
    void onBinaryMessageReceived(QByteArray);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;

    std::optional<QJsonObject> responseObj;
    void onWebcocketStateChanged();

public:
    bool sendMessage(QString text, QString chatId);
    bool logoutUser();
    QJsonArray searchMessage(QString message, QString chatId);
    bool createChat(QString name);
    bool getChatList(QList<Chat *> &chatList);
    // bool getMessageHistory(int chatId);
    void waitResponse();
    void sendRequest(const QJsonObject &requestObj);
    bool loginUser(QString login, QString password);
    bool registerUser(QString login, QString password);
};

#endif  // ECHOCLIENT_H
