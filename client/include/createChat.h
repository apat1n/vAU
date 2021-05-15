#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT
Q_SIGNALS:
    void requestCreating(QString name);

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_createChat_clicked();

private:
    Ui::Dialog *ui;
};

#endif  // DIALOG_H
