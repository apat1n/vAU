#include <QApplication>
#include <QtWebSockets/QtWebSockets>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    //    QCommandLineParser parser;
    //    parser.setApplicationDescription("vau Client");
    //    parser.addHelpOption();

    //    QCommandLineOption ipOption(
    //        QStringList() << "ip",
    //        QCoreApplication::translate(
    //            "main", "Ip of server output [default: localhost]."),
    //        QCoreApplication::translate("main", "port"),
    //        QLatin1String("localhost"));
    //    parser.addOption(ipOption);
    //    QCommandLineOption portOption(
    //        QStringList() << "p"
    //                      << "port",
    //        QCoreApplication::translate("main",
    //                                    "Port for echoserver [default:
    //                                    1234]."),
    //        QCoreApplication::translate("main", "port"),
    //        QLatin1String("1234"));
    //    parser.addOption(portOption);
    //    parser.process(a);

    //    QString ip = parser.value(ipOption);
    //    int port = parser.value(portOption).toInt();

    //    MainWindow w(QString("ws://%1:%2").arg(ip).arg(port));
    MainWindow w("ws://localhost:1234");
    w.show();
    return a.exec();
}
