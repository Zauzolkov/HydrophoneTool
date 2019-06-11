#ifndef STM32SONAR_H
#define STM32SONAR_H

#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QDataStream>
#include <QByteArray>
#include <QTimer>
#include <QSettings>
#include <QObject>

#include "data_types.h"
#include "server.h"

class stm32sonar : public QObject
{
    Q_OBJECT
public:
    explicit stm32sonar(QString configPath,
                        QObject *parent = nullptr);
    int success();

signals:
    QByteArray dataReceived();
    void readyToRead();

public slots:
    bool transmitSettings(settingsPacket settings);
    void handleReadyRead();
    void loadLastSettings();
    void saveLastSettings(settingsPacket settings);

private:
    QSerialPort *serialPort;
    QByteArray rxData;
    QSettings *sonarSettings;
    QTextStream  sOutput;
    Server *jsonServer;
    int counter = 0;
    bool toConsole = false;
    QTimer timer;
};

#endif // STM32SONAR_H
