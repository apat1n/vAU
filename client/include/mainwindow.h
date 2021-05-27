#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "chat.h"
#include "client.h"
#include "message.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(const QString &, QWidget * = nullptr);
    ~MainWindow();

Q_SIGNALS:
    void connectionUnstable();
private slots:

    void on_SendButton_clicked();

    void on_signIn_clicked();

    void on_pushButton_2_clicked();

    void on_registerButton_clicked();

    void on_signOut_clicked();

    void on_messageTextField_returnPressed();

    void on_search_textEdited(const QString &searchRequest);

    void on_chatMenu_itemClicked(QListWidgetItem *item);

    void on_createChatButton_clicked();

    void newChat(QString name);

    void renderMessages(int chat_id);

private:
    void onConnectionUnstable();
    void updateChats();
    void renderChats(const QMap<int, QSharedPointer<Chat>> &);

    Ui::MainWindow *ui;
    Client client;
    QMap<int, QSharedPointer<Chat>> availableChats;
    QMap<int, QString> availibleUsers;
};
#endif  // MAINWINDOW_H
