#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include <QDialog>
#include <QLabel>
#include <QList>
namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();

private:
    void ShowChatOrContact();
    void addChatUserList();

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    std::vector<QString>  strs ={"hello world !",
                                 "nice to meet u",
                                 "New year，new life",
                                "You have to love yourself",
                                "My love is written in the wind ever since the whole world is you"};

    std::vector<QString> heads = {
        ":/pic/resource/head_1.jpg",
        ":/pic/resource/head_2.jpg",
        ":/pic/resource/head_3.jpg",
        ":/pic/resource/head_4.jpg",
        ":/pic/resource/head_5.jpg"
    };

    std::vector<QString> names = {
        "llfc",
        "zack",
        "golang",
        "cpp",
        "java",
        "nodejs",
        "python",
        "rust"
    };
};

#endif // CHATDIALOG_H
