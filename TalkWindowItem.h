#pragma once

#include <QWidget>
#include "ui_TalkWindowItem.h"

class TalkWindowItem : public QWidget
{
	Q_OBJECT

public:
	TalkWindowItem(QWidget *parent = nullptr,QString uid= nullptr);
	~TalkWindowItem();
	
	void setHeadPixmap(const QPixmap& pixmap);	//设置头像
	void setMsgLabelContent(const QString& msg);//设置内容
	QString getMsgLabelText();					//返回文本
	QString getItemUid();						//拿取当前的id

	void setinitIndex(const QString& Name, const QString& image);//设置初始化数据信息
private :
	
signals:
	void signalCloseClicked(QString uid);	//点击删除信号

private:
	Ui::TalkWindowItemClass ui;

private :
	void initControl();	//初始化UI
	
	void enterEvent(QEnterEvent* event) override;	//进入事件
	void leaveEvent(QEvent* event)override;			//离开事件
	void resizeEvent(QResizeEvent* event)override;
private:
	QString m_uid;	//当前Item项的唯一ID
};

