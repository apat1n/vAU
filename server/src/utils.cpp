#include "QJsonObject"

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
