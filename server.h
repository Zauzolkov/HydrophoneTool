#pragma once

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
//#include <QWebSocket>
//#include <QWebSocketServer>
#include "data_types.h"


class QWebSocketServer;
class QWebSocket;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(quint16 port = 3005, QObject *parent = nullptr);
    bool success = false;

signals:
    void settingsReceived(settingsPacket settings);
    void closed();

public slots:
    void onNewConnection();
    void messageReceived(QString message);
    void sendResult(resultPacket &result, int id);
    void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    std::optional<settingsPacket> parseSettings(const QString data);
    bool m_debug;
};

