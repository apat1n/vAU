#include "database.h"
#include <sstream>

Database::Database(const QString &url,
                   int port,
                   const QString &name,
                   const QString &username,
                   const QString &password,
                   bool debug)
    : m_debug(debug) {
    db = QSqlDatabase::addDatabase("QPSQL");

    db.setHostName(url);
    db.setPort(port);
    db.setDatabaseName(name);
    db.setUserName(username);
    db.setPassword(password);

    if (m_debug) {
        qDebug() << "SQL connection: " << (db.open() ? "opened" : "closed");
    }
}

bool Database::authUser(User &user, QString password) {
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

bool Database::registerUser(QString login, QString password) {
    QSqlQuery query(db);
    std::stringstream ss;

    ss << "INSERT INTO QUser (login, password_hash) VALUES ('"
       << login.toStdString() << "', '" << password.toStdString() << "');";

    return query.exec(QString::fromStdString(ss.str()));
}

QList<Chat> Database::getChatList(User &user) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT id, name, is_group "
          "FROM QChat JOIN QChatUserList ON chat_id = id "
          "WHERE user_id = "
       << user.id << ";";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    query.exec(QString::fromStdString(ss.str()));

    QList<Chat> result;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        bool is_group = query.value(2).toBool();
        result.append(Chat{id, name, is_group, QDate()});
    }
    return result;
}

QList<Message> Database::getMessageList(int chatId) {
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

bool Database::createChat(QString name, int user_id, int &chat_id) {
    QSqlQuery query(db);
    std::stringstream ss;

    ss << "INSERT INTO QChat (name, creator_id) VALUES ('" << name.toStdString()
       << "', " << user_id << ");";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    if (query.exec(QString::fromStdString(ss.str()))) {
        chat_id = query.lastInsertId().toInt();
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

bool Database::createMessage(int chat_id,
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

QMap<int, QString> Database::getUserList(int chatId) {
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

QList<User> Database::getUserContacts(int user_id) {
    QSqlQuery query(db);
    std::stringstream ss;
    ss << "SELECT QUser.id, login FROM QUserContacts join QUser on quser.id = "
          "user_id2 "
          "where user_id1 = "
       << user_id << ";";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }
    query.exec(QString::fromStdString(ss.str()));

    QList<User> result;
    while (query.next()) {
        int user_id = query.value(0).toInt();
        QString login = query.value(1).toString();
        result.append({user_id, login});
    }

    return result;
}

bool Database::addUserContact(int user_id1, int user_id2) {
    QSqlQuery query(db);
    std::stringstream ss;

    ss << "INSERT INTO QUserContacts (user_id1, user_id2) VALUES (" << user_id1
       << ", " << user_id2 << ");";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    return query.exec(QString::fromStdString(ss.str()));
}

bool Database::inviteUserChat(int user_id, int chat_id) {
    QSqlQuery query(db);
    std::stringstream ss;

    ss << "INSERT INTO QChatUserList (chat_id, user_id) VALUES (" << chat_id
       << ", " << user_id << ");";

    if (m_debug) {
        qDebug() << QString::fromStdString(ss.str());
    }

    return query.exec(QString::fromStdString(ss.str()));
}
