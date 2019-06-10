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

    connect(serialPort, &QSerialPort::readyRead, this, &stm32sonar::handleReadyRead);
    connect(jsonServer, &Server::settingsReceived, this, &stm32sonar::transmitSettings);

//    connect(&timer, &QTimer::timeout, this, &stm32sonar::transmitSettings);
//    timer.start(1500);

//    emit(transmitSettings());
}

/* 0 = ok
 * 1 = failed to open
 * 2 = not writable
 */

int stm32sonar::success()
{
    if (serialPort->isOpen() && serialPort->isWritable())
    {
        return 0;
    } else if (!serialPort->isOpen()) {
        return 1;
    } else if (!serialPort->isWritable()) {
        return 2;
    } else {
        return -1;
    }
}

void stm32sonar::handleReadyRead()
{
//    if (serialPort->bytesAvailable() < 48) {
//        return;
//    }

    rxData.append(serialPort->read(48));

    if (rxData.startsWith(dataHeader) && rxData.endsWith(dataFooter))
    {
//        sOutput << "DataReceived!" << endl;
//        sOutput << rxData << endl;
        QDataStream in(rxData);
        in.setByteOrder(QDataStream::LittleEndian);
        resultPacket result;

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

//        result.angle = *(double*)(&result.angle);

        /*
        4 гфон = 1 на плате
        1 гфон = 2 на плате
        */

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

        /* сделать таймер, который будет сбрасывать чтение,
         * если долго нет ответов
         */


        /*
         * 1 - 2 - 4
         * A   B   C
         *
         *
         *
         *
         * 1 - 2
         *     |
         *     4
         *
         */

        rxData.clear();
        serialPort->clear();
        counter++;

        emit jsonServer->sendResult(result, counter);
    }

    if (!rxData.startsWith(dataHeader))
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

    sOutput << "\n\t\t/// SETTINGS ARE SET: " << written << " " << settings.dataHeader << " ///\n" << endl;

    return true;
}

//bool stm32sonar::transmitSettings()
//{
////    timer.stop();

//    settingsPacket newSettings;
//    // rate = (512*khz)/index
//    // (512.0*35.5)/255

//    newSettings.dataHeader = dataHeader;
////    newSettings.sampleRate0 = 70.72f;
////    newSettings.sampleRate1 = 70.04f;
////    newSettings.sampleRate2 = 68.07f;
//    newSettings.sampleRate0 = (512.0*25.0)/182.0;
//    newSettings.sampleRate1 = (512.0*25.0)/182.0;
//    newSettings.sampleRate2 = (512.0*25.0)/182.0;
////    newSettings.threshold = 55000;
//    newSettings.threshold = 30000;
//    newSettings.max_a_b = 150;
//    newSettings.max_a_c = 150;
//    newSettings.min_a_b = 50;
//    newSettings.min_a_c = 50;
//    newSettings.min_b_c = 50;
//    newSettings.base_a_b = 150;
//    newSettings.dataFooter = dataFooter;

//    QByteArray buffer;
//    QDataStream out(&buffer, QIODevice::ReadWrite);
//    out.setByteOrder(QDataStream::LittleEndian);
//    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

//    out << newSettings.dataHeader
//        << newSettings.sampleRate0
//        << newSettings.sampleRate1
//        << newSettings.sampleRate2
//        << newSettings.threshold
//        << newSettings.max_a_b
//        << newSettings.max_a_c
//        << newSettings.min_a_b
//        << newSettings.min_a_c
//        << newSettings.min_b_c
//        << newSettings.base_a_b
//        << newSettings.dataFooter;

////    sOutput << buffer << endl;

//    int written = serialPort->write(buffer);
//    serialPort->waitForBytesWritten(5000);

//    sOutput << "\n\t\t/// SETTINGS ARE SET: " << written << " ///\n" << endl;

//    return true;
//}
