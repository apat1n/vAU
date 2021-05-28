#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

    void on_signIn_clicked();

    void on_pushButton_2_clicked();

    void on_registerButton_clicked();

    void on_messageTextField_returnPressed();

    void on_search_textEdited(const QString &searchRequest);

    void on_chatMenu_itemClicked(QListWidgetItem *item);

    void on_createChatButton_clicked();

    void newChat(QString name);

    void on_actionDark_Theme_triggered();

    void on_actionLog_Out_triggered();

    void on_actionMy_Profile_triggered();

    void inviteUser(int id);
    // todo: потом сделать этот метод или что-нибудь в этом роде) и кстати у
    // чатов тоже кажется должен быть свой Id..

    void on_friends_clicked();

    void on_messages_clicked();

    void on_friendList_itemDoubleClicked(QListWidgetItem *item);

private:
    void onConnectionUnstable();
    void updateChats();
    void updateUsers();
    void renderChats(const QList<Chat *> &);
    void renderMessages(Chat *);
    void updateUserProfile(int id);

    Ui::MainWindow *ui;
    Client client;
    QList<Chat *> availableChats;
    QMap<int, QString> availibleUsers;
};
#endif  // MAINWINDOW_H
