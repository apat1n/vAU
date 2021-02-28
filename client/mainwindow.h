#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(const QString &, QWidget * = nullptr);
    void updateDebugText(const QString &);
    ~MainWindow();

private slots:

    void on_connectButton_clicked();

    void on_DisconnectButton_clicked();

    void on_StateButton_clicked();

    void on_SendButton_clicked();

    void on_signIn_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    Client client;
};
#endif  // MAINWINDOW_H
