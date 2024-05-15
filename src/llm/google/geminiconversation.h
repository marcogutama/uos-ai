#ifndef GEMINICONVERSATION_H
#define GEMINICONVERSATION_H

#include "conversation.h"

class GeminiConversation : public Conversation
{
public:
    GeminiConversation();

public:
    void update(const QByteArray &response);

public:
    static QJsonObject parseContentString(const QString &content);
    bool addUserData(const QString &data);
};

#endif // CONVERSATION_H
