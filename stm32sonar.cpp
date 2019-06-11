#include "stm32sonar.h"

stm32sonar::stm32sonar(QString devicePath,
                       int baudRate,
                       QObject *parent) : QObject(parent), sOutput(stdout)
{
    serialPort = new QSerialPort();
    serialPort->setPortName(devicePath);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::SoftwareControl);
    serialPort->open(QIODevice::ReadWrite);

    jsonServer = new Server(3005);

    counter = 0;

    connect(serialPort, &QSerialPort::readyRead,
            this, &stm32sonar::handleReadyRead);

    connect(jsonServer, &Server::settingsReceived,
            this, &stm32sonar::transmitSettings);
}

/* 0 = ok
 * 1 = failed to open
 * 2 = not writable
 * 3 = server failed to start
 */

int stm32sonar::success()
{
    if (serialPort->isOpen() && serialPort->isWritable() && jsonServer->success)
    {
        return 0;
    } else if (!serialPort->isOpen()) {
        return 1;
    } else if (!serialPort->isWritable()) {
        return 2;
    } else if (!jsonServer->success) {
        return 3;
    } else {
        return -1;
    }
}


void stm32sonar::handleReadyRead()
{
    rxData.append(serialPort->read(48));

    if (rxData.startsWith(dataHeader) && rxData.endsWith(dataFooter))
    {
        resultPacket result;
        QDataStream in(rxData);
        in.setByteOrder(QDataStream::LittleEndian);

        in  >> result.header
            >> result.frequency
            >> result.index
            >> result.amplitude
            >> result.firstHydrophone
            >> result.ping_quarter
            >> result.angle
            >> result.timeStamp0
            >> result.timeStamp1
            >> result.timeStamp2
            >> result.footer;

        sOutput << "OUTPUT: "
                << "\tangle: " << result.angle
                << "\tquart: " << result.ping_quarter
                << "\tfreq: "  << result.frequency
                << "  index: " << result.index
                << "  power: " << result.amplitude
                << "  first:"  << result.firstHydrophone
                << "  count: " << counter
                << "\n\ttime: " << result.timeStamp0
                << ", " << result.timeStamp1
                << ", " << result.timeStamp2
                << ".\n"
                << endl;

        rxData.clear();
        serialPort->clear();
        counter++;

        emit jsonServer->sendResult(result, counter);
    } else if (!rxData.startsWith(dataHeader))
    {
        rxData.clear();
        serialPort->clear();
    }
}


bool stm32sonar::transmitSettings(settingsPacket settings)
{
    // rate = (512*khz)/index
    // (512.0*35.5)/255

    settings.dataHeader = dataHeader;
    settings.dataFooter = dataFooter;

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::ReadWrite);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    out << settings.dataHeader
        << settings.sampleRate0
        << settings.sampleRate1
        << settings.sampleRate2
        << settings.threshold
        << settings.max_a_b
        << settings.max_a_c
        << settings.min_a_b
        << settings.min_a_c
        << settings.min_b_c
        << settings.base_a_b
        << settings.dataFooter;

    int written = serialPort->write(buffer);
    serialPort->waitForBytesWritten(5000);

    sOutput << "\n\t\t/// SETTINGS ARE SET: "
            << written << " "
            << settings.dataHeader << " ///\n" << endl;

    return true;
}

