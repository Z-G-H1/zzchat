#include "registerdialog.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "nomal");
    repolish(ui->err_tip);

    // 连接注册完成信号和槽函数
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

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
        showTip(tr("邮箱地址不正确"),false);
    }
}


void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err){
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        return;
    }

    // 解析json 字符串， res需要转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return ;
    }

    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return ;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 调用对应的逻辑
    _handlers[id](jsonDoc.object());

    return ;
}



void RegisterDialog::showTip(QString str, bool b_ok){
    if( b_ok ){
        ui->err_tip->setProperty("state","normal");
    }else{
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::initHttpHandlers(){
    // 注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,[this](QJsonObject jsonObj){
       int error = jsonObj["error"].toInt();
       if( error != ErrorCodes::SUCCESS ){
           showTip(tr("参数错误"),false);
           return ;
       }
       auto email = jsonObj["email"].toString();
       showTip(tr("验证码已经发送到邮箱，注意查收"), true);
       qDebug() << "email is " << email ;
    });
}
