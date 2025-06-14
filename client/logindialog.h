#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
public slots:
    void slot_forget_pwd();

signals:
    void switchRegister();
    void switchReset();

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
