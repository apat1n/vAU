#include <QApplication>
#include <QtWebSockets/QtWebSockets>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w("ws://localhost:1234");
    w.show();
    return a.exec();
}
