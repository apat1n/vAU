#include <sstream>
#include "server.h"

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

bool Server::registerUser(QString login, QString password) {
    QSqlQuery query(db);
    std::stringstream ss;

    ss << "INSERT INTO QUser (login, password_hash) VALUES ('"
       << login.toStdString() << "', '" << password.toStdString() << "');";

    return query.exec(QString::fromStdString(ss.str()));
}

QList<Chat> Server::getChatList(User &user) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT id, name FROM QChat JOIN QChatUserList ON chat_id = id WHERE "
          "user_id = "
       << user.id << ";";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

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

QList<Message> Server::getMessageList(int chatId) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT id, user_id, message, message_date FROM QChatMessages WHERE "
          "chat_id = "
       << chatId << ";";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    QList<Message> result;
    if (query.exec(QString::fromStdString(ss.str()))) {
        while (query.next()) {
            int id = query.value(0).toInt();
            int user_id = query.value(1).toInt();
            QString message =
                QByteArray::fromBase64(query.value(2).toByteArray());
            //        QDate lastUpdated = query.value(3).toDate();
            result.append(Message{id, user_id, message, QDate()});
        }
    }

    return result;
}

bool Server::createChat(QString name, int user_id) {
    QSqlQuery query(db);
    std::stringstream ss;

    ss << "INSERT INTO QChat (name, creator_id) VALUES ('" << name.toStdString()
       << "', " << user_id << ");";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    if (query.exec(QString::fromStdString(ss.str()))) {
        int chat_id = query.lastInsertId().toInt();
        ss.str("");  // clear query string
        ss << "INSERT INTO QChatUserList (chat_id, user_id) VALUES (" << chat_id
           << ", " << user_id << ");";
        if (m_debug) {
            qDebug() << QString::fromStdString(ss.str());
        }
        return query.exec(QString::fromStdString(ss.str()));
    }
    return false;
}

bool Server::createMessage(int chat_id,
                           int user_id,
                           QString message,
                           QDate date) {
    QSqlQuery query(db);
    std::stringstream ss;

    // Encoding to Base64
    QByteArray ba_message(std::move(message.toStdString().c_str()));

    ss << "INSERT INTO QChatMessages (chat_id, user_id, message, message_date) "
       << "VALUES (" << chat_id << ", " << user_id << ", '"
       << ba_message.toBase64().toStdString() << "', '"
       << date.toString(Qt::DefaultLocaleShortDate).toStdString() << "');";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    return query.exec(QString::fromStdString(ss.str()));
}

QMap<int, QString> Server::getUserList(int chatId) {
    QSqlQuery query(db);
    std::stringstream ss;
    if (chatId == -1) {
        ss << "SELECT id, login FROM quser;";
    } else {
        ss << "SELECT id, login FROM qchatuserlist JOIN quser ON quser.id = "
              "qchatuserlist.user_id WHERE chat_id = "
           << chatId << ";";
    }

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    query.exec(QString::fromStdString(ss.str()));

    QMap<int, QString> result;
    while (query.next()) {
        int user_id = query.value(0).toInt();
        QString user_name = query.value(1).toString();
        result[user_id] = user_name;
    }

    return result;
}
