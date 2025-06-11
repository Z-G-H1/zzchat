#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>

TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent), _counter(10)
{
    _timer = new QTimer(this);

    connect(_timer, &QTimer::timeout, [this](){
       _counter--;
       if(_counter <= 0){
           _timer->stop();
           _counter = 10;
           this->setText("获取");
           this->setEnabled(true);
           return ;
       }
       this->setText(QString::number(_counter));
    });
}


TimerBtn::~TimerBtn()
{
    _timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton) {
        // 在这里处理鼠标左键释放事件
        qDebug() << "MyButton was released!";
        this->setEnabled(false);    // 按钮置为不可用
        this->setText(QString::number(_counter));
//        _timer 对象开始以1000毫秒（即1秒）为间隔，
//        周期性地发出我们之前在构造函数里连接好的 timeout 信号
        _timer->start(1000);
        emit clicked();
    }
    // 调用基类的mouseReleaseEvent 以确保正常的事件处理
    QPushButton::mouseReleaseEvent(e);
}
