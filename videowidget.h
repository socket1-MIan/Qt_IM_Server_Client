#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
# include "TitleBar.h"
#include "basicwindow.h"

# include <QMouseEvent>
# include <QTimer>
# include <QDialog>

namespace Ui {
class VideoWidget;
}

class VideoWidget : public BasicWindow
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
    QWidget* getBtnWidget(); //返回视频窗口
    QVBoxLayout* getVBoxLayout();//返回布局
    QPushButton* getStopBtn();  //返回控件按钮

public slots:
    void on_closeBtn_clicked(); //点击挂断通话
    void on_show(); //显示窗口
    void on_hide(); //隐藏窗口


private:
    bool event(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

public :
     Ui::VideoWidget *ui;
private:



};

#endif // VIDEOWIDGET_H
