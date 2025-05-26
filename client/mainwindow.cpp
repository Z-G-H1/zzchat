#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 创建一个centralWidget, 并将其设置为mainWindow的中心部件
    _login_dlg = new LoginDialog();
    setCentralWidget(_login_dlg);
    _login_dlg->show();

    // 创建和注册消息的连接
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    _reg_dlg = new RegisterDialog();

}
void MainWindow::SlotSwitchReg(){
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}

MainWindow::~MainWindow()
{
    delete ui;
    if(_login_dlg){
        delete _login_dlg;
        _login_dlg = nullptr;
    }
    if(_reg_dlg){
        delete _reg_dlg;
        _reg_dlg = nullptr;
    }
}

