#pragma once

#include <QWidget>
#include "ui_TalkWindowShell.h"

#include "Basicwindow.h"
# include "EmotionWindow.h"
# include "TalkWindowItem.h"
# include  "ContactItem.h"

# include <QSqlDatabase>
# include <QSqlQuery>
# include <QTcpSocket>
# include <QUdpSocket>


class TalkWindow;

class TalkWindowShell : public BasicWindow
{
	Q_OBJECT

public:
    TalkWindowShell(QWidget* parent = nullptr);
	~TalkWindowShell();

public:
	//添加聊天页面
	void addTalkWindow(TalkWindowItem* item, const QString& uid);
	
	void setCurrentWidget(QWidget* widget);		//设置当前聊天部件
	void addContactWidget(const QString& uid, 	//添加聊天列表
							const QString& Name,
							const QString& Sign,
							const QString& Image,
							const QMap<QString, QVariantMap>& params);

	const QMap<QListWidgetItem*, QString>& getTalkWindowItemMap()const;
	QMap<QString, QVariantMap> getGroupMap(const QString& uid);//拿取 群成员数据
	QListWidgetItem* getListWidgetItem(const QString& uid);		//拿取列表成员Item项

private:
	void initControl();

public slots:
	//表情按键点击事件
	void onEmotionBtnClicked(bool);

private slots:
	//左侧列表点击后执行得槽函数
	void onTalkWindowItemClicked(QListWidgetItem* item);
	//选择得表情
	void onEmotionItemClicked(int emotionNum);

signals:
	

public:
	Ui::TalkWindowShellClass ui;

	// 打开得聊天窗口得映射
	QMap<QListWidgetItem*, QString> m_talkWindowItemMap;
	EmotionWindow* m_enotionWindow; //表情窗口

private:
    TalkWindow* m_TalkWindows;			//聊天页面

	QMap<QString, QMap<QString, QVariantMap>> m_GroupId;	//当前左侧聊天列表添加的群聊
	QMap<QString, QString> m_MemberName;					//当前左侧聊天列表与聊页面名字的映射

};

