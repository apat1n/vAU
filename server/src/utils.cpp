#include <QDir>
#include <QImage>
#include "QJsonObject"
#include <QBuffer>

[[nodiscard]] static QString imageToBase64(const QImage &image) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    image.save(&buffer,
               "PNG");  // writes the image in PNG format inside the buffer
    return QString::fromUtf8(byteArray.toBase64().data());
}

static void saveImage(const QImage &image, const QString &filename) {
    QDir dirPath = QDir::currentPath() + "/" + "images";
    if (!dirPath.exists()) {
        dirPath.mkpath(".");
    }
    image.save(dirPath.absolutePath() + "/" + filename);
}

static QImage loadImage(const QString &filename) {
    if (!QDir(QDir::currentPath() + "/" + "images" + "/" + filename).exists()) {
        throw std::runtime_error("no such image!");
    }
    return QImage(QDir::currentPath() + "/" + "images" + "/" + filename);
}

// For creating response with method and status
[[nodiscard]] static QJsonObject getJsonResponseInstance(QString &&method,
                                                         int status) {
    /*
     * {
     *  "responseObj": [
     *   {
     *    "response": [
     *     {
     *      "method": method,
     *      "status": status
     *     }
     *    ]
     *   }
     *  ]
     * }
     */

    QJsonObject response;
    response["method"] = method;
    response["status"] = status;

    QJsonObject responseObj;
    responseObj["response"] = response;
    return responseObj;
}

// For creating response with method, content and status
[[nodiscard]] static QJsonObject getJsonResponseInstance(QString &&method,
                                                         QJsonArray &&content,
                                                         int status) {
    /*
     * {
     *  "responseObj": [
     *   {
     *    "response": [
     *     {
     *      "method": method,
     *      "status": status,
     *      "message": [
     *       {
     *        "content": content (QJsonArray in this context)
     *       }
     *      ]
     *     }
     *    ]
     *   }
     *  ]
     * }
     */

    QJsonObject message;
    message["content"] = content;

    QJsonObject response;
    response["method"] = method;
    response["status"] = status;
    response["message"] = message;

    QJsonObject responseObj;
    responseObj["response"] = response;
    return responseObj;
}

// For creating response with method, content and status
[[nodiscard]] static QJsonObject getJsonResponseInstance(QString &&method,
                                                         QJsonObject &&content,
                                                         int status) {
    /*
     * {
     *  "responseObj": [
     *   {
     *    "response": [
     *     {
     *      "method": method,
     *      "status": status,
     *      "message": [
     *       {
     *        "content": content (QJsonObject in this context)
     *       }
     *      ]
     *     }
     *    ]
     *   }
     *  ]
     * }
     */

    QJsonObject message;
    message["content"] = content;

    QJsonObject response;
    response["method"] = method;
    response["status"] = status;
    response["message"] = message;

    QJsonObject responseObj;
    responseObj["response"] = response;
    return responseObj;
}
