#include "chatdialog.h"
#include "ui_chatdialog.h"
#include "QRandomGenerator"
#include "chatuserwid.h"

ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),_b_loading(false)
{
    ui->setupUi(this);
    ui->add_btn->SetState("normal", "hover", "press");
    ui->add_btn->setProperty("state", "normal");
    ShowChatOrContact();
    addChatUserList();

}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::ShowChatOrContact()
{

    if(_state == ChatUIMode::ChatMode){
           ui->chat_user_list->show();
           ui->con_user_list->hide();
           _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
           ui->chat_user_list->hide();
           ui->con_user_list->show();
           _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}
