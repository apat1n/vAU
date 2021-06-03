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

static QString getUserStatus(int id) {
    return "_status_";
}

static QIcon getChatImage(int id) {
    QPixmap pixmap(1000, 1000);
    pixmap.fill(Qt::red);
    return QIcon(pixmap);
}

static QImage getUserImage(int id, Client &client) {
    QImage photo;
    if (!client.getUserPhoto(photo, id)) {
        photo = QImage(256, 256, QImage::Format_RGB32);
        photo.fill(Qt::blue);
    }
    return photo;
}

static void saveImage(const QImage &image, const QString &filename) {
    QDir dirPath = QDir::currentPath() + "/" + "images";
    if (!dirPath.exists()) {
        dirPath.mkpath(".");
    }
    image.save(dirPath.absolutePath() + "/" + filename);
}

static QImage loadImage(const QString &filename) {
    return QImage(QDir::currentPath() + "/" + "images" + "/" + filename);
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
