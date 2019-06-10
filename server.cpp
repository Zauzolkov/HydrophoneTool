#include "server.h"
#include <QWebSocket>
#include <QWebSocketServer>
//#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>

Server::Server(quint16 port, QObject *parent) : QObject(parent)
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Sonar Server"),
                                              QWebSocketServer::NonSecureMode,
                                              this);

    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
    qDebug() << "Sonar Server listening on port" << port;

    connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
            this, &Server::onNewConnection);

    connect(m_pWebSocketServer, &QWebSocketServer::closed,
            this, &Server::closed);
   }

}


void Server::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &Server::messageReceived);
    connect(pSocket, &QWebSocket::disconnected, this, &Server::socketDisconnected);

    m_clients << pSocket;

//    pSocket->sendTextMessage("Welcome to SonarServer!");
}


void Server::messageReceived(QString message)
{
    QJsonDocument document = QJsonDocument::fromJson(message.toLocal8Bit());
    QJsonObject jsonObject = document.object();

    QString packetType = jsonObject["packetType"].toString();

    if (packetType == "settingsPacket")
    {
        settingsPacket settings;

        QJsonArray sampleRates = jsonObject["sampleRates"].toArray();

        settings.sampleRate0 = sampleRates[0].toDouble();
        settings.sampleRate1 = sampleRates[1].toDouble();
        settings.sampleRate2 = sampleRates[2].toDouble();
        settings.threshold = jsonObject["threshold"].toInt();
        settings.max_a_b  = jsonObject["max_a_b"].toInt();
        settings.max_a_c  = jsonObject["max_a_c"].toInt();
        settings.min_a_b  = jsonObject["min_a_b"].toInt();
        settings.min_a_c  = jsonObject["min_a_c"].toInt();
        settings.min_b_c  = jsonObject["min_b_c"].toInt();
        settings.base_a_b = jsonObject["base_a_b"].toInt();

        emit settingsReceived(settings);
    }
}


void Server::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
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
    jsonObject["firstHydrophone"] = qint32(result.firstHydrophone);
    jsonObject["quarter"] = qint32(result.ping_quarter);
    jsonObject["angle"] = result.angle;
    jsonObject["timeStamps"] = timeStamps;

    QJsonDocument jsonDoc(jsonObject);

//    QWebSocket *client;
//    foreach (client, m_clients) {
//        client->sendTextMessage();
//    }
    for (const auto &c : m_clients) {
        c->sendTextMessage(jsonDoc.toJson(QJsonDocument::Indented));
    }
}
