#include "geminiconversation.h"

#include <QJsonDocument>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QString>
#include <QDebug>

GeminiConversation::GeminiConversation()
{

}

QJsonObject GeminiConversation::parseContentString(const QString &trama)
{
    QJsonObject response;

    // Buscar la parte de la trama después de "data:"
    QRegularExpression regex(R"(data:\s*(.*))");
    QRegularExpressionMatch match = regex.match(trama);
    if (!match.hasMatch()) {
        qDebug() << "No se encontró la trama JSON después de 'data:'";
        return response;
    }

    QString jsonTrama = match.captured(1);

    // Crear un objeto QJsonDocument a partir de la trama JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonTrama.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Error al analizar la trama JSON:" << error.errorString();
        return response;
    }

    // Obtener el objeto JSON raíz
    QJsonObject rootObj = doc.object();

    // Obtener el arreglo "candidates"
    QJsonArray candidates = rootObj["candidates"].toArray();
    if (candidates.isEmpty()) {
        qDebug() << "No se encontró el arreglo 'candidates'";
        return response;
    }

    // Obtener el primer objeto del arreglo "candidates"
    QJsonObject candidateObj = candidates[0].toObject();

    // Obtener el objeto "content"
    QJsonObject contentObj = candidateObj["content"].toObject();
    QString role = contentObj["role"].toString();
    //qDebug() << "Role:" << role;
    response["role"] = role;

    // Obtener el arreglo "parts"
    QJsonArray partsArray = contentObj["parts"].toArray();
    if (!partsArray.isEmpty()) {
        QJsonObject partObj = partsArray[0].toObject();
        QString text = partObj["text"].toString();
        //qDebug() << "Text:" << text;
        response["content"] = text;
    }
    //
    // QString finishReason = candidateObj["finishReason"].toString();
    // qDebug() << "Finish Reason:" << finishReason;
    //
    // int index = candidateObj["index"].toInt();
    // qDebug() << "Index:" << index;
    //
    // // Obtener el arreglo "safetyRatings"
    // QJsonArray safetyRatingsArray = candidateObj["safetyRatings"].toArray();
    // for (const QJsonValue& rating : safetyRatingsArray) {
    //     QJsonObject ratingObj = rating.toObject();
    //     QString category = ratingObj["category"].toString();
    //     QString probability = ratingObj["probability"].toString();
    //     qDebug() << "Category:" << category << "Probability:" << probability;
    // }
    //
    // // Obtener el objeto "usageMetadata"
    // QJsonObject usageMetadataObj = rootObj["usageMetadata"].toObject();
    // int promptTokenCount = usageMetadataObj["promptTokenCount"].toInt();
    // int candidatesTokenCount = usageMetadataObj["candidatesTokenCount"].toInt();
    // int totalTokenCount = usageMetadataObj["totalTokenCount"].toInt();
    // qDebug() << "Prompt Token Count:" << promptTokenCount;
    // qDebug() << "Candidates Token Count:" << candidatesTokenCount;
    // qDebug() << "Total Token Count:" << totalTokenCount;

    return response;
}

void GeminiConversation::update(const QByteArray &response)
{
    if (response.isEmpty())
        return;

    if (response.startsWith("data:")) {
        const QJsonObject &delateData = parseContentString(response);
        if (delateData.contains("content")) {
            m_conversion.push_back(QJsonObject({
                { "role",     delateData.value("role")    },
                { "content",  delateData.value("content") }
            }));
        }
    }
}

bool GeminiConversation::addUserData(const QString &data)
{
    if (!data.isEmpty()) {
        const QJsonDocument &document = QJsonDocument::fromJson(data.toUtf8());
        if (document.isArray()) {
            QJsonArray jsonGemini;
            for (const QJsonValue &valor: document.array()) {
                if (valor.isObject()) {
                    QJsonObject elemento = valor.toObject();
                    QJsonArray a;
                    a.push_back(QJsonObject({{"text", elemento["content"]}}));
                    elemento["parts"] = a;
                    elemento.remove("content");
                    if (QString::compare("assistant", elemento["role"].toString()) == 0) {
                        elemento["role"] = "model";
                    }
                    jsonGemini.append(elemento);
                }
            }
            m_conversion = jsonGemini;
        } else {
            QJsonArray textContent;
            textContent.push_back(QJsonObject({ { "text", data} }));
            m_conversion.push_back(QJsonObject({ { "role", "user" }, {"parts", textContent} }));
        }
        return true;
    }
    return false;
}
