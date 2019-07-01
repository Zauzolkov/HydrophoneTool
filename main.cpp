#include <QCoreApplication>
#include <QSerialPortInfo>
#include <QObject>
#include <QTextStream>
#include <QSettings>
#include <QDir>
#include "stm32sonar.h"

//const QString configPath = QDir::currentPath() + "/SonarConfig.ini";
const QString configPath = "/etc/auv/config/sonar.ini";

const QString red    = "\033[1;31m";
const QString green  = "\033[1;32m";
const QString yellow = "\033[1;33m";
const QString norm   = "\033[0m";       // no color
const QString redError = red + "ERROR: " + norm;

void writeDefaultSettings(QSettings *settings)
{
    settings->setValue("UART/PID", 24577);
    settings->setValue("UART/VID", 1027);
    settings->setValue("UART/Serial", "A506MJ51"); // внимание! сер. номер - строка!
    settings->setValue("UART/BaudRate", 115200);
    settings->setValue("UART/LastPath", "ttyUSB0");

//    settings->setValue("LastSettings/SampleRate0", 70.5);
//    settings->setValue("LastSettings/SampleRate1", 70.0);
//    settings->setValue("LastSettings/SampleRate2", 70.0);

    //////////
    // калибровка:
    // rate = (512*khz)/index
    // (512.0*35.5)/255

    float curFreq = 35.5;
    double rates[] = {
        (512.0*curFreq)/260.5,
        (512.0*curFreq)/263.5,
        (512.0*curFreq)/266.5
    };

    settings->setValue("LastSettings/SampleRate0", QString::number(rates[0],'f',8));
    settings->setValue("LastSettings/SampleRate1", QString::number(rates[1],'f',8));
    settings->setValue("LastSettings/SampleRate2", QString::number(rates[2],'f',8));
    //////////

    settings->setValue("LastSettings/threshold", 35000);
    settings->setValue("LastSettings/max_a_b", 15);
    settings->setValue("LastSettings/max_a_c", 15);
    settings->setValue("LastSettings/min_a_b", 5);
    settings->setValue("LastSettings/min_a_c", 5);
    settings->setValue("LastSettings/min_b_c", 5);
    settings->setValue("LastSettings/base_a_b", 150);

    settings->setValue("ServerPort", 3005);
    settings->setValue("ConsoleOutput", true);

    settings->sync();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream stdOutput(stdout);
    QTextStream errOutput(stderr);

    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    QSettings settings(configPath, QSettings::IniFormat);

    if (!settings.contains("UART/PID"))
    {
        stdOutput << yellow << "Config file not found!" << norm << endl
                  << "Generating new config with default values..." << endl;
        writeDefaultSettings(&settings);
    }

    if (settings.status() == QSettings::AccessError)
    {
        stdOutput << red << "ERROR: Failed to write config!" << endl
                  << "Check your permissions!" << norm << endl;
    }

    bool toConsole = settings.value("ConsoleOutput").toBool();

    bool hasFoundSonar = false;
    for (const QSerialPortInfo &port : serialPortInfos) {
        if (port.productIdentifier() == settings.value("UART/PID").toInt() &&
            port.vendorIdentifier() == settings.value("UART/VID").toInt()  &&
            port.serialNumber() == settings.value("UART/Serial").toString())
        {
            if (toConsole)
                stdOutput << "Path of sonar is "
                          << yellow << port.portName() << norm << endl;
            settings.setValue("UART/LastPath", port.portName());
            hasFoundSonar = true;
        }
    }

    if (!hasFoundSonar)
    {
        if (toConsole)
            errOutput << redError << "Sonar not found!" << endl;
        std::abort();
//        settings.setValue("UART/LastPath", "ttyUSB0");
//        settings.sync();
    }

    stm32sonar sonar(configPath);

    if (toConsole)
    {
        switch (sonar.success())
        {
        case 0:
            stdOutput << green << "Success: Connected to sonar!" << norm  << endl;
            break;
        case 1:
            errOutput << redError
                      << "Failed to open serial device! " << endl
                      << "General failure reading your sonar." << endl;
            break;
        case 2:
            errOutput << redError
                      << "Serial device is not writable!" << endl
                      << "Check your permissions!" << endl;
            break;
        case 3:
            errOutput << redError
                      << "Failed to start WebSocket server!" << endl
                      << "Maybe something is already running on this port." << endl;
            break;
        default:
            errOutput << red << "ERROR: UNKNOWN FAILURE!" << endl;
            break;
        }
    }

    if (sonar.success() != 0)
        std::abort();

    return QCoreApplication::exec();
}
