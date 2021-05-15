#include <chat.h>
#include <QDebug>
#include <QJsonObject>
#include <QList>
#include <QListWidget>
#include <QString>
#include <algorithm>
#include <vector>

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

[[nodiscard]] static QList<Chat *> foundMatches(const QList<Chat *> input,
                                                const QString &pattern) {
    QList<Chat *> result;
    for (auto *chat : input) {
        if (isMatch(chat->getName(), pattern)) {
            result.append(chat);
        }
    }
    return result;
}

static void clearListWidget(QListWidget *listWidget) {
    while (listWidget->count() > 0) {
        listWidget->takeItem(0);
    }
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