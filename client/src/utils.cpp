#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QListWidget>
#include <QString>
#include <algorithm>
#include <vector>
#include "chat.h"
#include "client.h"

[[nodiscard]] static bool isMatch(const QString &message,
                                  const QString &pattern) {
    QString s = pattern + "&" + message;
    qDebug() << s;
    int n = s.size();

    std::vector<int> z(n, 0);

    int l = 0, r = 0;

    for (int i = 1; i < n; i++) {
        if (i <= r) {
            z[i] = std::min(r - i + 1, z[i - l]);
        }
        while (z[i] + i < n && s[z[i]] == s[z[i] + i]) {
            z[i]++;
        }
        if (z[i] + i - 1 > r) {
            l = i, r = z[i] + i - 1;
        }
    }
    return *std::max_element(z.begin(), z.end()) == pattern.size();
}

[[nodiscard]] static QMap<int, QSharedPointer<Chat>> foundMatches(
    const QMap<int, QSharedPointer<Chat>> &input,
    const QString &pattern) {
    QMap<int, QSharedPointer<Chat>> result;
    for (QMap<int, QSharedPointer<Chat>>::const_iterator chat =
             input.constBegin();
         chat != input.constEnd(); ++chat) {
        if (isMatch(chat.value()->getName(), pattern)) {
            result.insert(chat.key(), chat.value());
        }
    }
    return result;
}

[[nodiscard]] static QString imageToBase64(const QImage &image) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    image.save(&buffer,
               "PNG");  // writes the image in PNG format inside the buffer
    return QString::fromUtf8(byteArray.toBase64().data());
}

static void clearListWidget(QListWidget *listWidget) {
    while (listWidget->count() > 0) {
        listWidget->takeItem(0);
    }
}

static void saveImage(const QImage &image, const QString &filename) {
    QDir dirPath = QDir::currentPath() + "/" + "images";
    if (!dirPath.exists()) {
        dirPath.mkpath(".");
    }
    image.save(dirPath.absolutePath() + "/" + filename, "PNG");
}

static QImage loadImage(const QString &filename) {
    QString filePath = QDir::currentPath() + "/" + "images" + "/" + filename;
    if (!QFile::exists(filePath)) {
        throw std::runtime_error("no such image!");
    }
    return QImage(filePath);
}

static QImage getUserImage(int id, Client &client) {
    QImage photo;
    QString filePath = QString("user_original_%1").arg(id);
    try {
        photo = loadImage(filePath);
        qDebug() << "load cached image";
    } catch (const std::exception &e) {
        qDebug() << "load image from server";
        if (!client.getUserPhoto(photo, id)) {
            photo = QImage(256, 256, QImage::Format_RGB32);
            photo.fill(Qt::blue);
        }
        saveImage(photo, filePath);
    }
    return photo;
}

static QIcon getChatImage(int id, Client &client) {
    QMap<int, QString> users;
    client.getUserList(users, id);
    for (QMap<int, QString>::iterator it = users.begin(); it != users.end();
         ++it) {
        if (it.key() != client.getId()) {
            return QPixmap::fromImage(getUserImage(it.key(), client));
        }
    }
    QPixmap pixmap(1000, 1000);
    pixmap.fill(Qt::blue);
    return QIcon(pixmap);
}

/*
 * Methods below using in creating QJsonObject that will be sent to server
 */

// For creating response with method
[[nodiscard]] static QJsonObject getJsonRequestInstance(const QString &method) {
    /*
     * {
     *  "requestObj": [
     *   {
     *    "request": [
     *     {
     *      "method": method,
     *     }
     *    ]
     *   }
     *  ]
     * }
     */

    QJsonObject request;
    request["method"] = method;

    QJsonObject requestObj;
    requestObj["request"] = request;
    return requestObj;
}

// For creating response with method and content
[[nodiscard]] static QJsonObject getJsonRequestInstance(const QString &method,
                                                        QJsonObject &&message) {
    /*
     * {
     *  "requestObj": [
     *   {
     *    "request": [
     *     {
     *      "method": method,
     *      "message": message
     *     }
     *    ]
     *   }
     *  ]
     * }
     */

    QJsonObject request;
    request["method"] = method;
    request["message"] = message;

    QJsonObject requestObj;
    requestObj["request"] = request;
    return requestObj;
}
