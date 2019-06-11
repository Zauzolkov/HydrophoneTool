#include "stm32sonar.h"

stm32sonar::stm32sonar(QString configPath,
                       QObject *parent) : QObject(parent), sOutput(stdout)
{
    sonarSettings = new QSettings(configPath, QSettings::IniFormat);

    toConsole = sonarSettings->value("ConsoleOutput").toBool();

    serialPort = new QSerialPort();
    serialPort->setPortName(sonarSettings->value("UART/LastPath").toString());
    serialPort->setBaudRate(sonarSettings->value("UART/BaudRate").toInt());
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::SoftwareControl);
    serialPort->open(QIODevice::ReadWrite);

    jsonServer = new Server(sonarSettings->value("ServerPort").toInt());

    connect(serialPort, &QSerialPort::readyRead,
            this, &stm32sonar::handleReadyRead);

    connect(jsonServer, &Server::settingsReceived,
            this, &stm32sonar::transmitSettings);

    loadLastSettings();
}


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

        if (toConsole)
        {
            sOutput << "OUTPUT:"
                    << "\tangle: " << result.angle
                    << "\tquart: " << result.ping_quarter
                    << "  freq: "  << result.frequency
                    << "  index: " << result.index
                    << "  power: " << result.amplitude
                    << "  first:"  << result.firstHydrophone
                    << "  count: " << counter
                    << "\n\ttime: " << result.timeStamp0
                    << ", " << result.timeStamp1
                    << ", " << result.timeStamp2
                    << ".\n"
                    << endl;
        }

        rxData.clear();
        serialPort->clear();
        counter++;

        emit jsonServer->sendResult(result, counter);
    } else if (!rxData.startsWith(dataHeader)) {
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

    serialPort->write(buffer);
    serialPort->waitForBytesWritten(5000);

    if (toConsole)
        sOutput << "Settings were transmitted!" << endl;

    saveLastSettings(settings);

    return true;
}

void stm32sonar::loadLastSettings()
{
    settingsPacket settings;
    settings.sampleRate0 = sonarSettings->value("LastSettings/SampleRate0").toFloat();
    settings.sampleRate1 = sonarSettings->value("LastSettings/SampleRate1").toFloat();
    settings.sampleRate2 = sonarSettings->value("LastSettings/SampleRate2").toFloat();
    settings.threshold   = sonarSettings->value("LastSettings/threshold").toInt();
    settings.max_a_b     = sonarSettings->value("LastSettings/max_a_b").toInt();
    settings.max_a_c     = sonarSettings->value("LastSettings/max_a_c").toInt();
    settings.min_a_b     = sonarSettings->value("LastSettings/min_a_b").toInt();
    settings.min_a_c     = sonarSettings->value("LastSettings/min_a_c").toInt();
    settings.min_b_c     = sonarSettings->value("LastSettings/min_b_c").toInt();
    settings.base_a_b    = sonarSettings->value("LastSettings/base_a_b").toInt();

    emit transmitSettings(settings);
}


void stm32sonar::saveLastSettings(settingsPacket settings)
{
    sonarSettings->setValue("LastSettings/SampleRate0", settings.sampleRate0);
    sonarSettings->setValue("LastSettings/SampleRate1", settings.sampleRate1);
    sonarSettings->setValue("LastSettings/SampleRate2", settings.sampleRate2);
    sonarSettings->setValue("LastSettings/threshold", settings.threshold);
    sonarSettings->setValue("LastSettings/max_a_b", settings.max_a_b);
    sonarSettings->setValue("LastSettings/max_a_c", settings.max_a_c);
    sonarSettings->setValue("LastSettings/min_a_b", settings.min_a_b);
    sonarSettings->setValue("LastSettings/min_a_c", settings.min_a_c);
    sonarSettings->setValue("LastSettings/min_b_c", settings.min_b_c);
    sonarSettings->setValue("LastSettings/base_a_b", settings.base_a_b);
    sonarSettings->sync();
}
