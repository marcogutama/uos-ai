#include "ollamachatcompletion.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

OllamaChatCompletion::OllamaChatCompletion(const AccountProxy &account)
    : OllamaNetWork(account)
{

}

QPair<int, QString> OllamaChatCompletion::create(const QString &model, AIConversation &conversation, qreal temperature)
{
    QJsonObject dataObject;
    dataObject.insert("model", model);
    dataObject.insert("messages", conversation.getConversions());
    dataObject.insert("temperature", qBound(0.0, temperature, 2.0));
    dataObject.insert("stream", true);

    if (!conversation.getFunctions().isEmpty()) {
        dataObject.insert("functions", conversation.getFunctions());
    }

    const QPair<int, QByteArray> &resultPairs = request(dataObject, "/chat/completions");

    if (resultPairs.first != 0)
        return qMakePair(resultPairs.first, resultPairs.second);

    conversation.update(resultPairs.second);
    return qMakePair(0, QString());
}
