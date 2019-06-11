#include <QCoreApplication>
#include <QSerialPortInfo>
#include <QObject>
#include <QTextStream>
#include <QSettings>
#include <QDir>
#include "stm32sonar.h"

void writeDefaultSettings(QSettings *settings, QSerialPortInfo *port)
{
    settings->setValue("UART/PID", port->productIdentifier());
    settings->setValue("UART/VID", port->vendorIdentifier());
    settings->setValue("UART/Serial", port->serialNumber());
    settings->setValue("UART/BaudRate", 115200);
    settings->setValue("UART/LastPath", port->portName());

    settings->setValue("LastSettings/SampleRate0", 70.5);
    settings->setValue("LastSettings/SampleRate1", 70.0);
    settings->setValue("LastSettings/SampleRate2", 71.2);
    settings->setValue("LastSettings/threshold", 45000);
    settings->setValue("ServerPort", 3005);
    settings->setValue("ConsoleOutput", true);
    settings->sync();
}

QString hex(int val)
{
    return QString::number(val, 16).toUpper();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream stdOutput(stdout);
    QTextStream errOutput(stderr);
    QTextStream stdInput(stdin);

    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    QSettings settings(QDir::currentPath() + "/SonarConfig.ini",
                       QSettings::IniFormat);

    if (!settings.contains("UART/PID"))
    {
        QString response;

        stdOutput << "Welcome to SonarService!" << endl
                  << "There is no any configured sonar." << endl
                  << "Would you like to generate new config? (Y/N)" << endl;

        stdInput >> response;

        if (response.toLower() != "y")
        {
            std::exit(0);
        } else {
            stdOutput << "\nAvailable serial ports:" << endl << endl;
            int counter = 0;
            for (const QSerialPortInfo &port : serialPortInfos) {
                counter++;
                stdOutput << "[ " << counter << " ]"
                          << "\t" << port.portName() << endl
                          << "Manuf.:\t" << port.manufacturer() << endl
                          << "Descr.:\t" << port.description() << endl
                          << "PID:\t" << hex(port.productIdentifier()) << endl
                          << "VID:\t" << hex(port.vendorIdentifier()) << endl
                          << "Serial:\t" << port.serialNumber() << endl
                          << endl;
            }

            do {
                stdOutput << "Enter the number of desired device"
                          << "[1-"<< counter << "]:" << endl;

                stdInput >> response;

            } while(response.toInt() < 1 || response.toInt() > counter );

            QSerialPortInfo chosenPort = serialPortInfos[response.toInt()-1];

            stdOutput << "\nOk, saving the config..." << endl;

            writeDefaultSettings(&settings, &chosenPort);

        }
    }

    bool hasFoundSonar = false;
    for (const QSerialPortInfo &port : serialPortInfos) {
        if (port.productIdentifier() == settings.value("UART/PID").toInt() &&
            port.vendorIdentifier() == settings.value("UART/VID").toInt()  &&
            port.serialNumber() == settings.value("UART/Serial").toString())
        {
            stdOutput << "Path of sonar is " << port.portName() << endl;
            settings.setValue("UART/LastPath", port.portName());
            hasFoundSonar = true;
        }
    }

    if (!hasFoundSonar)
    {
        errOutput << "ERROR: Sonar not found!" << endl;
        std::abort();
    }

    // отправлять в stm32sonar qsettings?
    // чтобы оттуда читать все нужные настройки уже внутри

    stm32sonar sonar(QString("ttyUSB0"), 115200);

    switch (sonar.success())
    {
    case 0:
        stdOutput << "Success: SonarService started!" << endl;
        break;
    case 1:
        errOutput << "ERROR: Failed to open serial device! " << endl
                  << "General failure reading your sonar." << endl;
        break;
    case 2:
        errOutput << "ERROR: Serial device is not writable!" << endl
                  << "Check your permissions!" << endl;
        break;
    case 3:
        errOutput << "ERROR: Failed to start WebSocket server!" << endl
                  << "Maybe something is already running on 3005 port." << endl;
        break;
    default:
        errOutput << "ERROR: UNKNOWN FAILURE!" << endl;
        break;
    }

    if (sonar.success() != 0)
        std::abort();

    return QCoreApplication::exec();
}
