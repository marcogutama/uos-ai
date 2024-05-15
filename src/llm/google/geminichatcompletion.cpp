#include "geminichatcompletion.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GeminiChatCompletion::GeminiChatCompletion(const AccountProxy &account)
    : GeminiNetWork(account)
{

}

QPair<int, QString> GeminiChatCompletion::create(const QString &model, GeminiConversation &conversation, qreal temperature)
{
    QJsonObject dataObject;
    dataObject.insert("contents", conversation.getConversions());

    const QPair<int, QByteArray> &resultPairs =
            request(dataObject, "/v1beta/models/" + model.toLower() + ":streamGenerateContent?alt=sse&key=");

    if (resultPairs.first != 0)
        return qMakePair(resultPairs.first, resultPairs.second);

    conversation.update(resultPairs.second);
    return qMakePair(0, QString());
}
