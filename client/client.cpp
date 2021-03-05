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

// void Client::sendMessage(const QString &message) {
//    m_webSocket.sendTextMessage(message);

//    QByteArray ba;

//    m_webSocket.sendBinaryMessage(ba);
//}

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
    qDebug() << message;
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
    qDebug() << requestBinaryMessage;

    m_webSocket.sendBinaryMessage(requestBinaryMessage);
    waitResponse();
}

bool Client::sendMessage(Message *messageObj, int chatId) {
    QJsonObject message;
    message["content"] = messageObj->getText();
    message["chatId"] = chatId;

    QJsonObject request;
    request["message"] = message;
    request["method"] = "sendMessage";

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    return true;
    //    if (responseObj->empty()) {
    //        return false;
    //    }

    //    QJsonObject responseBody = responseObj->value("response").toObject();
    //    QString method = responseBody.value("method").toString();
    //    int status = responseBody.value("status").toInt();
    //    responseObj.reset();
    //    qDebug() << "response was got";
    //    return method == request["method"].toString() && status == 200;
}

bool Client::createChat(QString name) {
    QJsonObject message;
    message["name"] = name;

    QJsonObject request;
    request["method"] = "createChat";
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == request["method"].toString() && status == 200;
}

bool Client::getChatList(QList<Chat *> &chatList) {
    QJsonObject message;

    QJsonObject request;
    request["method"] = "getChatList";
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    if (method == request["method"].toString() && status == 200) {
        QJsonArray responseArray =
            responseBody.value("message").toObject().value("content").toArray();
        for (auto chatData : responseArray) {
            int id = chatData.toObject().value("id").toInt();
            QString name = chatData.toObject().value("name").toString();
            chatList.append(new Chat(id, name));
        }
    }

    return method == request["method"].toString() && status == 200;
}

bool Client::getChatMessages(int chatId, QList<Message *> &messageHistory) {
    QJsonObject content;
    content["chat_id"] = chatId;

    QJsonObject message;
    message["content"] = content;

    QJsonObject request;
    request["method"] = "getChatMessages";
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    if (method == request["method"].toString() && status == 200) {
        QJsonArray responseArray =
            responseBody.value("message").toObject().value("content").toArray();
        for (auto responseMessage : responseArray) {
            int id = responseMessage.toObject().value("id").toInt();
            QString messageText =
                responseMessage.toObject().value("message_text").toString();
            messageHistory.append(new Message(messageText, id));
        }
        //        qDebug() << "Got " << messageHistory.size() << " messages!";
    }

    return method == request["method"].toString() && status == 200;
}

bool Client::logoutUser() {
    QJsonObject request;
    request["method"] = "logout";
    request["message"] = "";

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    responseObj.reset();

    return method == request["method"].toString() && status == 200;
}

bool Client::loginUser(QString login, QString password) {
    QJsonObject message;
    message["login"] = login;
    message["password"] = password;

    QJsonObject request;
    request["method"] = "login";
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    currId = responseBody.value("id").toInt();

    responseObj.reset();
    return method == request["method"].toString() && status == 200;
}

bool Client::registerUser(QString login, QString password) {
    QJsonObject message;
    message["login"] = login;
    message["password"] = password;

    QJsonObject request;
    request["method"] = "register";
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;

    sendRequest(requestObj);
    if (responseObj->empty()) {
        return false;
    }

    QJsonObject responseBody = responseObj->value("response").toObject();
    QString method = responseBody.value("method").toString();
    int status = responseBody.value("status").toInt();
    currId = responseBody.value("id").toInt();

    responseObj.reset();

    return method == request["method"].toString() && status == 200;
}
