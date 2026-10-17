#include "ContactItem.h"

# include <QPainter>

ContactItem::ContactItem(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initControl(); 
}

ContactItem::~ContactItem()
{}
//修改用户名
void ContactItem::setUserName(const QString & userName)
{
	ui.userName->setText(userName);
}
//修改个性签名
void ContactItem::setSignName(const QString& sginName)
{
	ui.signName->setText(sginName);
}
//头像
void ContactItem::setHeadPixamp(const QPixmap& headPath)
{
	ui.label->setPixmap(headPath);
}

QString ContactItem::getUserName() const
{
	return ui.userName->text();
}

QSize ContactItem::getHeadLabelSize() const
{
	return ui.label->size();
}

//设置未读消息数量
void ContactItem::setUnreadCount(int unreadCnt)
{

    m_unreadCount =  unreadCnt;
    update();// 触发重绘事件

}

void ContactItem::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if(m_unreadCount <= 0)
        return; //未读为0，直接不画角标

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); //抗锯齿，圆形平滑

    //控件右上角区域，角标大小
    int badgeW = 18;
    int badgeH = 18;
    int x = this->width() - badgeW - 12; //距离右边6像素边距
    int y = 4; //距离上边4像素

    QRectF badgeRect(x,y,badgeW,badgeH);
    //填充红色圆形背景
    painter.setBrush(QColor(255,60,60));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(badgeRect);

    //绘制数字，超过9显示"9+"
    QString text;
    if(m_unreadCount >9)
        text = "9+";
    else
        text = QString::number(m_unreadCount);

    painter.setPen(Qt::white);
    QFont f = painter.font();
    f.setPixelSize(10);
    painter.setFont(f);
    painter.drawText(badgeRect,Qt::AlignCenter, text);

}

void ContactItem::initControl()
{
}
