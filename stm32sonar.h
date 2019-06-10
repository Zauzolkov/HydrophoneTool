#ifndef STM32SONAR_H
#define STM32SONAR_H

#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QDataStream>
#include <QByteArray>
#include <QTimer>
#include <QObject>

const uint32_t dataHeader = 'A';
const uint32_t dataFooter = '\n';

struct resultPacket {
    uint32_t header;
    uint32_t frequency;
    uint32_t index;
    uint32_t amplitude;
    uint32_t firstHydrophone;
    int32_t ping_quarter;
    double angle; // 8 bytes
    uint32_t timeStamp0;
    uint32_t timeStamp1;
    uint32_t timeStamp2;
    uint32_t footer;
};

struct settingsPacket {
    uint32_t dataHeader;
    float sampleRate0;
    float sampleRate1;
    float sampleRate2;
    uint32_t threshold;
    int32_t max_a_b;
    int32_t max_a_c;
    int32_t min_a_b;
    int32_t min_a_c;
    int32_t min_b_c;
    int32_t base_a_b;
    uint32_t dataFooter;
};

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
    bool transmitSettings();
    void handleReadyRead();

public: // PRIVATE
    QSerialPort *serialPort;
    QByteArray   rxData;
    QTimer       timer;
    int          counter = 0;

private:
    QTextStream  sOutput;
};

#endif // STM32SONAR_H
