#pragma once

#include <QWidget>
#include "ui_ContactItem.h"

//联系人列表项
class ContactItem : public QWidget
{
	Q_OBJECT

public:
	ContactItem(QWidget *parent = nullptr);
	~ContactItem();

	void setUserName(const QString& userName);	//修改用户名
	void setSignName(const QString& sginName);	//修改个性签名
	void setHeadPixamp(const QPixmap& headPath);//头像
	QString getUserName()const;
    QSize getHeadLabelSize()const;

    void setUnreadCount(int unreadCnt); //设置未读消息数量

protected:
    void paintEvent(QPaintEvent* event) override;

private:
	void initControl(); 


private:
	Ui::ContactItemClass ui;
    int m_unreadCount = 0; //   未读消息计数， == 0 不显示红点
};

