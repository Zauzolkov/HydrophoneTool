#include <QCoreApplication>
#include <QObject>
#include <QTextStream>
#include "stm32sonar.h"
#include <QDebug>
#include <QSerialPortInfo>

//#include <optional>

//std::optional<settingsPacket> parse_some(const QString val) {
//    if (true) {
//        return settingsPacket{};
//    } else {
//        std::nullopt;
//    }
//}

int main(int argc, char *argv[])
{
//    qInfo() << "Info";
//    qWarning() << "Warn";
//    qDebug() << "Deb";
//    qFatal() << "Fatal err";
//    auto QSerialPortInfo::availablePorts().first().
    QCoreApplication a(argc, argv);
    QTextStream stOutput(stdout);

//    auto val = parse_some(QSting("blah"));
//    if (val != std::nullopt) {
//        val->get()...
//    }

    stOutput << "tada!" << endl;

    stm32sonar sonar(QString("ttyUSB0"), 115200);

    switch (sonar.success())
    {
    case 0:
        stOutput << "0: success!" << endl;
        break;
    case 1:
        stOutput << "1: FAILED TO OPEN DEVICE! general failure reading your sonar" << endl;
        break;
    case 2:
        stOutput << "2: DEVICE IS NOT WRITABLE! check your permissions!" << endl;
        break;
    default:
        stOutput << "!!! UNKNOWN ERROR !!!" << endl;
        break;
    }

//    sonar.rxData.append(">");
//    sonar.rxData.append(sonar.serialPort->readAll());
//    sonar.rxData.append(sonar.serialPort->readLine(100));

//    stOutput << sonar.rxData << endl;

//    return 0;
    return QCoreApplication::exec();
}
