#ifndef SERVERDEFS_H
#define SERVERDEFS_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDebug>

#include "uosaccountencoder.h"

enum SocketProxyType {
    SYSTEM_PROXY  = 0,  // 系统代理
    NO_PROXY      = 1,  // 无代理
    SOCKET_PROXY  = 3,  // SOCKET5代理
    HTTP_PROXY    = 4,  // HTTP代理
};

enum LLMChatModel {
    CHATGPT_3_5          = 0,   // GPT3.5
    CHATGPT_3_5_16K      = 1,   // GPT3.5 16k
    CHATGPT_4            = 2,   // GPT4
    CHATGPT_4_32K        = 3,   // GPT4 32k
    SPARKDESK            = 10,  // 迅飞星火1.5
    SPARKDESK_2          = 11,  // 迅飞星火2.0
    SPARKDESK_3          = 12,  // 迅飞星火3.0
    WXQF_ERNIE_Bot       = 20,  // 百度文心千帆ERNIE_Bot
    WXQF_ERNIE_Bot_turbo = 21,  // 百度文心千帆ERNIE_Bot_turbo
    WXQF_ERNIE_Bot_4     = 22,  // 百度文心千帆ERNIE_Bot_4
    GPT360_S2_V9         = 40,  // 360GPT
    ChatZHIPUGLM_PRO     = 50,  // 智谱AIPRO
    ChatZHIPUGLM_STD     = 51,  // 智谱AISTD
    ChatZHIPUGLM_LITE    = 52,  // 智谱AIpLITE
    OLLAMA               = 60,  // Ollama
    GEMINI               = 70,  // Gemini
    LOCAL_TEXT2IMAGE     = 1000,// 本地文本转图片模型
};

enum ModelType {
    USER = 0,           // 用户自己的账号
    FREE_NORMAL = 1,    // 普通免费账号
    FREE_KOL = 2,       // KOL免费账号
    LOCAL = 3,          // 本地模型
};

enum ChatAction {
    ChatTextPlain     = 0,      // 纯文本聊天
    ChatFunctionCall  = 1,      // FunctionCall
    ChatText2Image    = 2       // 文生图
};

struct SocketProxy {
    SocketProxyType socketProxyType = NO_PROXY; // 默认不使用代理
    QString user = "";                      // 服务器账户
    QString pass = "";                      // 服务器密码
    QString host = "";                      // 服务器地址
    quint16  port = 0;                      // 服务器端口号
    bool operator== (const SocketProxy &param) const
    {
        return (socketProxyType == param.socketProxyType && user == param.user
                && host == param.host && port == param.port);
    }

    QJsonObject toJson() const
    {
        QJsonObject objJson;
        objJson.insert("user", user);
        objJson.insert("pass", pass);
        objJson.insert("host", host);
        objJson.insert("port", port);
        objJson.insert("socketProxyType", socketProxyType);

        return objJson;
    }

    void fromJson(const QJsonObject &objJson)
    {
        user = objJson.value("user").toString();
        pass = objJson.value("pass").toString();
        host = objJson.value("host").toString();
        port = static_cast<quint16>(objJson.value("port").toInt());
        socketProxyType = static_cast<SocketProxyType>(objJson.value("socketProxyType").toInt());
    }
};

struct AccountProxy {
    QString appId;
    QString apiKey;
    QString apiSecret;

    SocketProxy socketProxy;

    QJsonObject toJson(bool decrypt = false) const
    {
        QJsonObject objJson;
        if (decrypt) {
            UosAccountEncoder encoder;
            objJson.insert("appId", std::get<0>(encoder.decrypt(appId)));
            objJson.insert("apiKey", std::get<0>(encoder.decrypt(apiKey)));
            objJson.insert("apiSecret", std::get<0>(encoder.decrypt(apiSecret)));
        } else {
            objJson.insert("appId", appId);
            objJson.insert("apiKey", apiKey);
            objJson.insert("apiSecret", apiSecret);
        }

        objJson.insert("socketProxy", socketProxy.toJson());
        return objJson;
    }

    void fromJson(const QJsonObject &objJson)
    {
        appId = objJson.value("appId").toString();
        apiKey = objJson.value("apiKey").toString();
        apiSecret = objJson.value("apiSecret").toString();
        socketProxy.fromJson(objJson.value("socketProxy").toObject());
    }

    static AccountProxy xfInlineAccount()
    {
        AccountProxy account;
        account.appId     = "ae3a30d6";
        account.apiKey    = "ca9af472727c12e347202d79f2b0f5a9";
        account.apiSecret = "MGEyMWVhYTRlYTZiNWI5NmE0ZTFkZjc0";
        return account;
    }
};

struct LLMServerProxy {
    QString id;
    QString name;

    LLMChatModel model;
    AccountProxy account;
    ModelType type;

    bool isValid() const
    {
        return !id.isEmpty() && !account.apiKey.isEmpty();
    }

    LLMServerProxy decryptAccount() const
    {
        LLMServerProxy llmAccount = *this;

        if (type == ModelType::FREE_NORMAL || type == ModelType::FREE_KOL) {
            UosAccountEncoder encoder;
            llmAccount.account.appId     = std::get<0>(encoder.decrypt(llmAccount.account.appId));
            llmAccount.account.apiKey    = std::get<0>(encoder.decrypt(llmAccount.account.apiKey));
            llmAccount.account.apiSecret = std::get<0>(encoder.decrypt(llmAccount.account.apiSecret));
        }

        return llmAccount;
    }

    QJsonObject toJson() const
    {
        QJsonObject objJson;
        objJson.insert("id",      id);
        objJson.insert("name",    name);
        objJson.insert("model",   model);
        objJson.insert("account", account.toJson(ModelType::FREE_NORMAL == type || ModelType::FREE_KOL == type));
        objJson.insert("type",    type);
        objJson.insert("icon",    llmName(model));
        return objJson;
    }

    void fromJson(const QJsonObject &objJson)
    {
        type = static_cast<ModelType>(objJson.value("type").toInt());
        model = static_cast<LLMChatModel>(objJson.value("model").toInt());
        account.fromJson(objJson.value("account").toObject());
        name = objJson.value("name").toString();
        id = objJson.value("id").toString();
    }

    QString toExpandString() const
    {
        QJsonObject objJson;
        objJson.insert("type", type);
        return QJsonDocument(objJson).toJson(QJsonDocument::Compact);
    }

    void fromExpandString(const QString &string)
    {
        QJsonDocument objDoc = QJsonDocument::fromJson(string.toUtf8());
        if (!objDoc.isNull()) {
            type = static_cast<ModelType>(objDoc.object().value("type").toInt());
        }
    }

    static QString llmName(LLMChatModel model)
    {
        switch (model) {
        case LLMChatModel::CHATGPT_3_5:
        case LLMChatModel::CHATGPT_3_5_16K:
            return QCoreApplication::translate("LLMServerProxy", "GPT3.5（OpenAI）");
        case LLMChatModel::CHATGPT_4:
        case LLMChatModel::CHATGPT_4_32K:
            return QCoreApplication::translate("LLMServerProxy", "GPT4（OpenAI）");
        case LLMChatModel::GPT360_S2_V9:
            return QCoreApplication::translate("LLMServerProxy", "360智脑");
        case LLMChatModel::SPARKDESK:
            return QCoreApplication::translate("LLMServerProxy", "星火大模型1.5（讯飞）");
        case LLMChatModel::SPARKDESK_2:
            return QCoreApplication::translate("LLMServerProxy", "星火大模型2.0（讯飞）");
        case LLMChatModel::SPARKDESK_3:
            return QCoreApplication::translate("LLMServerProxy", "星火大模型3.0（讯飞）");
        case LLMChatModel::WXQF_ERNIE_Bot:
            return QCoreApplication::translate("LLMServerProxy", "ERNIE-Bot");
        case LLMChatModel::WXQF_ERNIE_Bot_turbo:
            return QCoreApplication::translate("LLMServerProxy", "ERNIE-Bot-turbo");
        case LLMChatModel::WXQF_ERNIE_Bot_4:
            return QCoreApplication::translate("LLMServerProxy", "ERNIE-Bot-4");
        case LLMChatModel::ChatZHIPUGLM_PRO:
        case LLMChatModel::ChatZHIPUGLM_STD:
        case LLMChatModel::ChatZHIPUGLM_LITE:
            return QCoreApplication::translate("LLMServerProxy", "ChatGLM-turbo（智谱）");
        case LLMChatModel::OLLAMA:
            return QCoreApplication::translate("LLMServerProxy", "Ollama");
        case LLMChatModel::GEMINI:
            return QCoreApplication::translate("LLMServerProxy", "Gemini");
        case LLMChatModel::LOCAL_TEXT2IMAGE:
            return QCoreApplication::translate("LLMServerProxy", "TextToImage(Local)");
        }

        return QCoreApplication::translate("LLMServerProxy", "unknown model");
    }

    static QString llmIcon(LLMChatModel model)
    {
        QString icon = ":/assets/images/default.svg";
        QString name = "default.svg";

        switch (model) {
        case LLMChatModel::CHATGPT_3_5:
        case LLMChatModel::CHATGPT_3_5_16K:
        case LLMChatModel::CHATGPT_4:
        case LLMChatModel::CHATGPT_4_32K: {
            icon = ":/assets/images/openai.svg";
            name = "openai.svg";
            break;
        }
        case LLMChatModel::GPT360_S2_V9: {
            icon = ":/assets/images/360.svg";
            name = "360.svg";
            break;
        }
        case LLMChatModel::SPARKDESK:
        case LLMChatModel::SPARKDESK_2:
        case LLMChatModel::SPARKDESK_3: {
            icon = ":/assets/images/iflytek.svg";
            name = "iflytek.svg";
            break;
        }
        case LLMChatModel::WXQF_ERNIE_Bot:
        case LLMChatModel::WXQF_ERNIE_Bot_turbo:
        case LLMChatModel::WXQF_ERNIE_Bot_4: {
            icon = ":/assets/images/baidu.svg";
            name = "baidu.svg";
            break;
        }
        case LLMChatModel::ChatZHIPUGLM_PRO:
        case LLMChatModel::ChatZHIPUGLM_STD:
        case LLMChatModel::ChatZHIPUGLM_LITE: {
            icon = ":/assets/images/zhipu.svg";
            name = "zhipu.svg";
            break;
        }
        case LLMChatModel::OLLAMA: {
        icon = ":/assets/images/ollama.svg";
        name = "ollama.svg";
        break;
        }
        case LLMChatModel::GEMINI: {
        icon = ":/assets/images/gemini.svg";
        name = "gemini.svg";
        break;
        }
        case LLMChatModel::LOCAL_TEXT2IMAGE: {
            icon = ":/assets/images/textimage.svg";
            name = "textimage.svg";
            break;
        }
        default:
            break;
        }

        static QTemporaryDir m_tempDir;
        if (m_tempDir.isValid()) {
            QString tempFile = m_tempDir.path() + "/" + name;
            if (!QFile::exists(tempFile) && !QFile::copy(icon, tempFile)) {
                qWarning() << "copy llm icon error " << tempFile;
            }

            return tempFile;
        } else {
            qWarning() << "QTemporaryDir invalid " << m_tempDir.errorString();
        }

        return icon;
    }
};

#endif // SERVERDEFS_H
