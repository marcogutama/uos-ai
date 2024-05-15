#include "gemini.h"
#include "geminiconversation.h"
#include "geminichatcompletion.h"
#include "aiimages.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Gemini::Gemini(const LLMServerProxy &serverproxy)
    : LLM(serverproxy)
{

}

QJsonObject Gemini::predict(const QString &content, const QJsonArray &functions, const QString &systemRole, qreal temperature)
{
    GeminiConversation conversion;
    conversion.addUserData(content);
    //conversion.setFunctions(functions);

    if (!systemRole.isEmpty())
        conversion.setSystemData(systemRole);

    GeminiChatCompletion chatCompletion(m_accountProxy.account);
    connect(this, &Gemini::aborted, &chatCompletion, &GeminiChatCompletion::requestAborted);
    connect(&chatCompletion, &GeminiChatCompletion::readyReadDeltaContent, this, &Gemini::onReadyReadChatDeltaContent);

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
    connect(&chatCompletion, &GeminiChatCompletion::requestFinished, this, [&]() {
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

QList<QByteArray> Gemini::text2Image(const QString &prompt, int number)
{
    AIImages textToImage(m_accountProxy.account);
    connect(this, &Gemini::aborted, &textToImage, &AIImages::requestAborted);

    QList<QByteArray> imageData;
    QPair<int, QString> errorpair = textToImage.create(prompt, imageData, number);
    setLastError(errorpair.first);
    setLastErrorString(errorpair.second);

    return imageData;
}

QPair<int, QString> Gemini::verify()
{
    GeminiConversation conversion;
    conversion.addUserData("Account verification only, no need for any response.");

    GeminiChatCompletion chatCompletion(m_accountProxy.account);
    connect(this, &Gemini::aborted, &chatCompletion, &GeminiChatCompletion::requestAborted);

    QPair<int, QString> errorpair = chatCompletion.create(m_accountProxy.name, conversion);
    setLastError(errorpair.first);
    setLastErrorString(errorpair.second);

    return errorpair;
}

void Gemini::onReadyReadChatDeltaContent(const QByteArray &content)
{
    if (content.isEmpty() || !stream())
        return;
    const QJsonObject &deltacontent = GeminiConversation::parseContentString(content);

#ifdef OpenTextAuditService
    if (!deltacontent.isEmpty()) {
        m_tasMgr->auditText(deltacontent.toLocal8Bit());
    }
#else
    if (deltacontent.contains("content"))
        emit readyReadChatDeltaContent(deltacontent.value("content").toString());
#endif
}