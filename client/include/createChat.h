#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QListWidget>
#include "client.h"
namespace Ui {
class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT
Q_SIGNALS:
    void requestCreating(QString name);

    void requestAddUser(int id);
public:
    explicit Dialog(Client &client, QMap<int, QString> users_, QWidget *parent = nullptr);
    ~Dialog();
private slots:
    void on_createChat_clicked();

    void on_avaliableUsers_itemClicked(QListWidgetItem *item);

private:
    Ui::Dialog *ui;
    Client &client;
};

#endif  // DIALOG_H
