#ifndef UPDATEPROFILE_H
#define UPDATEPROFILE_H

#include <QDialog>
#include <QString>
#include "client.h"

namespace Ui {
class updateProfile;
}

class updateProfile : public QDialog {
    Q_OBJECT
Q_SIGNALS:
    void requestChangeLogin(QString login);
    void requestChangeStatus(QString status);

public:
    explicit updateProfile(QString login_,
                           QString status_,
                           QWidget *parent = nullptr);
    ~updateProfile();

private slots:
    void on_changeProfile_clicked();

private:
    Ui::updateProfile *ui;
    QString login;
    QString status;
};

#endif  // UPDATEPROFILE_H
