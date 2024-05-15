#ifndef GEMININETWORK_H
#define GEMININETWORK_H

#include "basenetwork.h"

class GeminiNetWork : public BaseNetWork
{
    Q_OBJECT

public:
    explicit GeminiNetWork(const AccountProxy &account);

public:
    /**
     * @brief rootUrlPath
     * @return
     */
    QString rootUrlPath() const;

    /**
     * @brief request
     * @param data
     * @param path
     * @param multipart
     * @return
     */
    QPair<int, QByteArray> request(const QJsonObject &data, const QString &path, QHttpMultiPart *multipart = nullptr);
};

#endif //GEMININETWORK_H
