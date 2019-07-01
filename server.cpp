#include "server.h"
#include <QWebSocket>
#include <QWebSocketServer>
#include <optional>


Server::Server(quint16 port, QObject *parent) : QObject(parent)
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Sonar Server"),
                                              QWebSocketServer::NonSecureMode,
                                              this);

    success = m_pWebSocketServer->listen(QHostAddress::Any, port);

    connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
            this, &Server::onNewConnection);

    connect(m_pWebSocketServer, &QWebSocketServer::closed,
            this, &Server::closed);
}


void Server::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived,
            this, &Server::messageReceived);

    connect(pSocket, &QWebSocket::disconnected,
            this, &Server::socketDisconnected);

    m_clients << pSocket;
}


void Server::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}


std::optional<settingsPacket> Server::parseSettings(const QString data) {
    QJsonDocument document = QJsonDocument::fromJson(data.toLocal8Bit());
    QJsonObject jsonObject = document.object();

    QStringList parameters;
    parameters << "base_a_b"
               << "max_a_b"
               << "max_a_c"
               << "min_a_b"
               << "min_a_c"
               << "min_b_c"
               << "packetType"
               << "sampleRates"
               << "threshold";

    if (jsonObject.keys() == parameters &&
        jsonObject["packetType"] == "settingsPacket")
    {
        settingsPacket settings;

        QJsonArray sampleRates = jsonObject["sampleRates"].toArray();

        settings.sampleRate0 = sampleRates[0].toDouble();
        settings.sampleRate1 = sampleRates[1].toDouble();
        settings.sampleRate2 = sampleRates[2].toDouble();
        settings.threshold = jsonObject["threshold"].toInt();
        settings.max_a_b = jsonObject["max_a_b"].toInt();
        settings.max_a_c = jsonObject["max_a_c"].toInt();
        settings.min_a_b = jsonObject["min_a_b"].toInt();
        settings.min_a_c = jsonObject["min_a_c"].toInt();
        settings.min_b_c = jsonObject["min_b_c"].toInt();
        settings.base_a_b = jsonObject["base_a_b"].toInt();

        return settings;
    }
    return std::nullopt;
}


void Server::messageReceived(QString message)
{
    auto settings = parseSettings(message);
    if (settings != std::nullopt) {
        emit settingsReceived(settings.value());
    }
}


void Server::sendResult(resultPacket &result, int id)
{
    QJsonObject jsonObject;

    QJsonArray timeStamps = {
        qint32(result.timeStamp0),
        qint32(result.timeStamp1),
        qint32(result.timeStamp2)
    };

    jsonObject["packetType"] = "resultPacket";
    jsonObject["packetId"] = id;
    jsonObject["frequency"] = qint32(result.frequency);
    jsonObject["rawIndex"] = qint32(result.index);
    jsonObject["amplitude"] = qint32(result.amplitude);
    jsonObject["lastHydrophone"] = qint32(result.lastHydrophone);
    jsonObject["quarter"] = qint32(result.ping_quarter);
    jsonObject["angle"] = result.angle;
    jsonObject["timeStamps"] = timeStamps;
    jsonObject["overPinger"] = result.overPinger;

    QJsonDocument jsonDoc(jsonObject);

    for (const auto &c : m_clients) {
        // TODO: в финальной версии заменить QJsonDocument::Indented на Compact
        c->sendTextMessage(jsonDoc.toJson(QJsonDocument::Indented));
    }
}
