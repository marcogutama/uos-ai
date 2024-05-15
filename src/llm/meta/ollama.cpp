#include "ollama.h"
#include "aiconversation.h"
#include "ollamachatcompletion.h"
#include "aiimages.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Ollama::Ollama(const LLMServerProxy &serverproxy)
    : LLM(serverproxy)
{

}

QJsonObject Ollama::predict(const QString &content, const QJsonArray &functions, const QString &systemRole, qreal temperature)
{
    AIConversation conversion;
    conversion.addUserData(content);
    conversion.setFunctions(functions);

    if (!systemRole.isEmpty())
        conversion.setSystemData(systemRole);

    OllamaChatCompletion chatCompletion(m_accountProxy.account);
    connect(this, &Ollama::aborted, &chatCompletion, &OllamaChatCompletion::requestAborted);
    connect(&chatCompletion, &OllamaChatCompletion::readyReadDeltaContent, this, &Ollama::onReadyReadChatDeltaContent);

#ifdef OpenTextAuditService
    connect(m_tasMgr.data(), &TasManager::sigAuditContentResult, this, [&](QSharedPointer<TextAuditResult> result) {
        if (result && result->code == TextAuditEnum::None) {
            emit readyReadChatDeltaContent(result->content);
        } else if (result->code == TextAuditEnum::NetError) {
            setLastError(AIServer::ErrorType::NetworkError);
            setLastErrorString(ServerCodeTranslation::serverCodeTranslation(lastError(), ""));
            emit aborted();
        } else {
            setLastError(AIServer::ErrorType::SenSitiveInfoError);
            setLastErrorString(ServerCodeTranslation::serverCodeTranslation(lastError(), ""));
            emit aborted();
        }
    });
    connect(&chatCompletion, &OllamaChatCompletion::requestFinished, this, [&]() {
        m_tasMgr->endAuditText();
    });
#endif

    QPair<int, QString> errorpair = chatCompletion.create(m_accountProxy.name, conversion, temperature);

#ifdef OpenTextAuditService
    do {
        TimerEventLoop oneloop;
        oneloop.setTimeout(1000);
        oneloop.exec();
        m_tasMgr->stopAuditing();
    } while (!m_tasMgr->auditFinished());
    if (lastError() != AIServer::ErrorType::NetworkError && lastError() != AIServer::ErrorType::SenSitiveInfoError) {
        setLastError(errorpair.first);
        setLastErrorString(ServerCodeTranslation::serverCodeTranslation(lastError(), errorpair.second));
    }
#else
    setLastError(errorpair.first);
    setLastErrorString(errorpair.second);
#endif

    QJsonObject response;
    response["content"] = conversion.getLastResponse();

    QJsonObject tools = conversion.getLastTools();
    if (!tools.isEmpty()) {
        response["tools"] = tools;
    }
    return response;
}

QList<QByteArray> Ollama::text2Image(const QString &prompt, int number)
{
    AIImages textToImage(m_accountProxy.account);
    connect(this, &Ollama::aborted, &textToImage, &AIImages::requestAborted);

    QList<QByteArray> imageData;
    QPair<int, QString> errorpair = textToImage.create(prompt, imageData, number);
    setLastError(errorpair.first);
    setLastErrorString(errorpair.second);

    return imageData;
}

QPair<int, QString> Ollama::verify()
{
    AIConversation conversion;
    conversion.addUserData("Account verification only, no need for any response.");

    OllamaChatCompletion chatCompletion(m_accountProxy.account);
    connect(this, &Ollama::aborted, &chatCompletion, &OllamaChatCompletion::requestAborted);

    QPair<int, QString> errorpair = chatCompletion.create(m_accountProxy.name, conversion);
    setLastError(errorpair.first);
    setLastErrorString(errorpair.second);

    return errorpair;
}

void Ollama::onReadyReadChatDeltaContent(const QByteArray &content)
{
    if (content.isEmpty() || !stream())
        return;
    const QJsonObject &deltacontent = AIConversation::parseContentString(content);

#ifdef OpenTextAuditService
    if (!deltacontent.isEmpty()) {
        m_tasMgr->auditText(deltacontent.toLocal8Bit());
    }
#else
    if (deltacontent.contains("content"))
        emit readyReadChatDeltaContent(deltacontent.value("content").toString());
#endif
}