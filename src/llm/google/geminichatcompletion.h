#ifndef GEMINICHATCOMPLETION_H
#define GEMINICHATCOMPLETION_H

#include "gemininetwork.h"
#include "geminiconversation.h"

class GeminiChatCompletion : public GeminiNetWork
{
public:
    explicit GeminiChatCompletion(const AccountProxy &account);

    QPair<int, QString> create(const QString &model, GeminiConversation &conversation, qreal temperature = 1.0);
};

#endif //GEMINICHATCOMPLETION_H
