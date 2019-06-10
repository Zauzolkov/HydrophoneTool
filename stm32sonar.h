#ifndef STM32SONAR_H
#define STM32SONAR_H

#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QDataStream>
#include <QByteArray>
#include <QTimer>
#include <QObject>

#include "data_types.h"
#include "server.h"

class stm32sonar : public QObject
{
    Q_OBJECT
public:
    explicit stm32sonar(QString devicePath = "ttyUSB0",
                        int baudRate = 115200,
                        QObject *parent = nullptr);
    int success();

signals:
    QByteArray dataReceived();
    void readyToRead();

public slots:
    bool transmitSettings(settingsPacket settings);
    void handleReadyRead();

public: // make it PRIVATE later
    QSerialPort *serialPort;
    QByteArray   rxData;
    QTimer       timer;
    int          counter = 0;

private:
    QTextStream  sOutput;
    Server *jsonServer;
};

#endif // STM32SONAR_H
