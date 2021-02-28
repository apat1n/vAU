#include "client.h"

QT_USE_NAMESPACE

Client::Client(const QUrl &url, bool debug, QObject *parent)
    : QObject(parent), m_url(url), m_debug(debug) {
    if (m_debug) {
        qDebug() << "WebSocket server:" << url;
    }

    connect(&m_webSocket, &QWebSocket::connected, this, &Client::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Client::closed);
}

void Client::connectServer() {
    m_webSocket.open(QUrl(m_url));
}

void Client::disconnectServer() {
    m_webSocket.close();
}

void Client::sendMessage(const QString &message) {
    m_webSocket.sendTextMessage(message);

    QByteArray ba;

    m_webSocket.sendBinaryMessage(ba);
}

QString Client::getState() const {
    using SocketState = QAbstractSocket::SocketState;
    SocketState state = m_webSocket.state();
    switch (state) {
        case SocketState::ConnectedState:
            return "OPEN";
            break;
        case SocketState::ConnectingState:
            return "Connecting";
            break;
        case SocketState::ClosingState:
            return "Closing";
            break;
        case SocketState::UnconnectedState:
            return "Closed";
            break;
    }
    return "Unknown";
}

void Client::onConnected() {
    if (m_debug) {
        qDebug() << "WebSocket connected";
    }
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this,
            &Client::onBinaryMessageReceived);
}

void Client::onBinaryMessageReceived(QByteArray message) {
    qDebug() << message;
    responseObj = QJsonDocument::fromJson(message).object();
    emit responseRecieved();
}

bool Client::login(QString login, QString password) {
    QJsonObject message;
    message["login"] = login;
    message["password"] = password;

    QJsonObject request;
    request["method"] = "login";
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;

    QByteArray requestBinaryMessage = QJsonDocument(requestObj).toJson();
    qDebug() << requestBinaryMessage;

    m_webSocket.sendBinaryMessage(requestBinaryMessage);

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &Client::responseRecieved, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(5000);  // wait for 5s to get response
    loop.exec();

    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("request").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    return method == "login" && status == 200;
}
