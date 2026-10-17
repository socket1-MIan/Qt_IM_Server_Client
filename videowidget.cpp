#include "videowidget.h"
#include "ui_videowidget.h"

VideoWidget::VideoWidget(QWidget *parent)
    : BasicWindow(parent)
    , ui(new Ui::VideoWidget)
{
    ui->setupUi(this);
    //this->setButtonType(ButtonType::MIN_BUTTON); //设置窗口按钮类型
    ui->widget_2->setStyleSheet("background: transparent; border: none;");
    ui->widget_2->hide();
    ui->BtnWidget->setStyleSheet("background-color: rgb(140, 140, 140);");
    // 给外层widget_2一个测试背景颜色，用来确认它渲染出来
    ui->widget->setStyleSheet("background-color:rgba(255,0,0,95);");



    this->hide();
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

QWidget* VideoWidget::getBtnWidget()
{

    return ui->BtnWidget;
}

//返回布局
QVBoxLayout *VideoWidget::getVBoxLayout()
{
    return ui->VBBtnWidget;
}

//返回控件按钮
QPushButton *VideoWidget::getStopBtn()
{
    return ui->SotpBtn;
}

//挂断通话
void VideoWidget::on_closeBtn_clicked()
{

}

//显示窗口
void VideoWidget::on_show()
{
    this->show();
}

//隐藏窗口
void VideoWidget::on_hide()
{
    this->hide();
}

bool VideoWidget::event(QEvent *event)
{

   return BasicWindow::event(event);

}

void VideoWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    qDebug()<<"====鼠标进入窗口====";
    ui->widget_2->show();
    ui->widget_2->raise();


}

void VideoWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    qDebug()<<"====鼠标离开窗口====";
    ui->widget_2->hide();
}



