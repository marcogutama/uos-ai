#ifndef OLLAMANETWORK_H
#define OLLAMANETWORK_H

#include "basenetwork.h"

class OllamaNetWork : public BaseNetWork
{
    Q_OBJECT

public:
    explicit OllamaNetWork(const AccountProxy &account);

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

#endif //OLLAMANETWORK_H
