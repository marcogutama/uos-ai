#ifndef OLLAMACHATCOMPLETION_H
#define OLLAMACHATCOMPLETION_H

#include "ollamanetwork.h"
#include "aiconversation.h"

class OllamaChatCompletion : public OllamaNetWork
{
public:
    explicit OllamaChatCompletion(const AccountProxy &account);

    QPair<int, QString> create(const QString &model, AIConversation &conversation, qreal temperature = 1.0);
};

#endif //OLLAMACHATCOMPLETION_H
