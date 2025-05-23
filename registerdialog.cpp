#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include <QRegularExpression>
#include <QtDebug>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "nomal");
    repolish(ui->err_tip);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    // 验证邮箱地址的正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配

    if( match ){
        // 发送http验证码
        qDebug() << "邮箱验证通过，发送验证码";
        ui->err_tip->setText("");
        ui->err_tip->setProperty("state", "normal");
        repolish(ui->err_tip);
    }else {
        // 提示邮箱不正确
        showTip(tr("邮箱地址不正确"));
    }
}

void RegisterDialog::showTip(QString str){
    ui->err_tip->setText(str);
    ui->err_tip->setProperty("state", "err");
    repolish(ui->err_tip);

}
