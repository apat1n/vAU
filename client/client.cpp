#include "client.h"

QT_USE_NAMESPACE

#include <QJsonArray>

Client::Client(const QUrl &url, bool debug, QObject *parent)
    : QObject(parent), m_url(url), m_debug(debug) {
    if (m_debug) {
        qDebug() << "WebSocket server:" << url;
    }

    connect(&m_webSocket, &QWebSocket::connected, this, &Client::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Client::closed);
    connect(&m_webSocket, &QWebSocket::stateChanged, this,
            &Client::onWebcocketStateChanged);
}

void Client::connectServer() {
    m_webSocket.open(QUrl(m_url));
}

void Client::disconnectServer() {
    m_webSocket.close();
}

int Client::getId() const {
    return currId;
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

// QJsonArray Client::searchMessage(QString message, QString chatId){

//}

void Client::onConnected() {
    if (m_debug) {
        qDebug() << "WebSocket connected";
    }
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this,
            &Client::onBinaryMessageReceived);
}

void Client::onBinaryMessageReceived(QByteArray message) {
    if (m_debug) {
        qDebug() << message;
    }

    responseObj = QJsonDocument::fromJson(message).object();
    emit responseRecieved();
}

void Client::onWebcocketStateChanged() {
    // if connection unstable
    if (m_webSocket.state() != QAbstractSocket::SocketState::ConnectedState) {
        emit connectionUnstable();
    }
}

void Client::waitResponse() {
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &Client::responseRecieved, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(5000);  // wait for 5s to get response
    loop.exec();
}

void Client::sendRequest(const QJsonObject &requestObj) {
    while (m_webSocket.state() !=
           QAbstractSocket::SocketState::ConnectedState) {
        connectServer();
        if (m_webSocket.state() !=
            QAbstractSocket::SocketState::ConnectedState) {
            QTimer timer;
            timer.setSingleShot(true);
            QEventLoop loop;
            connect(&m_webSocket, &QWebSocket::connected, &loop,
                    &QEventLoop::quit);
            connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            timer.start(1000);  // wait for 1s to connect server
            loop.exec();
        }
    }

    QByteArray requestBinaryMessage = QJsonDocument(requestObj).toJson();

    if (m_debug) {
        qDebug() << requestBinaryMessage;
    }

    m_webSocket.sendBinaryMessage(requestBinaryMessage);
    waitResponse();
}
