#pragma once

#include <QWidget>

#include "ui_TalkWindow.h"
# include "TalkWindowShell.h"
#include "SendFile.h"
#include "ChatListModel.h"
#include "ChatMsgDelegate.h"
#include "videowidget.h"
#include "administratorwidget.h"

# include <QFileDialog>
# include <QListView>

class TalkWindow : public QWidget
{
	Q_OBJECT
		
public:

   TalkWindow(QWidget* parent = nullptr);

	~TalkWindow();

	void addEmotionImage(int emotionNum);		//添加表情图片
	void setWindowName(const QString& name);	//设置窗口名称
	QString getTalkID();						//获取TalkID
	void setWidgetUI(const QString& uid,		//修改窗口的UI来进行不同的联系人切换效果
					const QMap<QString, QVariantMap>& ListGroup);		

	void MessageSwitchFile(const QString& uid);	//将当前这个聊天页面写入本地文件缓存

private slots:
	//发送
	void onSendBtnClicked(bool);
	//发送文件按键被点击了
	void onFileOpenBtnClicked(bool);
	//和群成员进行单聊
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
	//关闭按键点击
	void on_closeBtn_clicked();
private:
	void intControl();
	void initGroupTalkStatus();		//判断是否为多人聊天
	int getCompDepID();				//获取公司部门的ID号

	void initTalkWindow_Server();	//初始化多人群聊
	//单聊
	void initPtoPTiak();
	//添加名称
	void addPeopInfo(QTreeWidgetItem* pRootGroupItem , int employeeID);
	//添加窗口子项
	void addPeopInfo(QTreeWidgetItem* pRootGroupItem,
		const QString& uId,
		const QString& Name,
		const QString& Sign,
		const QString& image );
	
	void slotContactClicked(const QString& contactId);//切换联系人视图
	//接收到对方消息时
	void addRecvMsg(const ChatMsg& Message);
	void slotSendBtnClicked(const QString& Naber  ,		//我发送消息
							const ChatMsg::MsgType& msgType , 
							const QString& path ,
							const QString& uid);	

	void inintListView();		//初始化视图模型代理
    void RequestVide();         //进行通话请求

private:
	Ui::TalkWindow ui;
	QString m_talkId;	//当前的id
	bool m_isFroupTalk; //判断当前的聊天是多人还是单人

	QMap<QString, QVariantMap> m_MapGroupp; //当前群聊所有的成员数据
	QMap<QString, QString> m_MapGroupImage; //群聊成员的手机号与头像数据的映射

	//GroupType m_groupType;
	QMap<QTreeWidgetItem*, QString> m_groupPeopleMap; //所有分组联系人姓名

	/*---- 视图模型代理 -----*/
	QListView* m_listview;		//视图
	ChatListModel* m_msgModel; //模型
	ChatMsgDelegate* m_delegate;//代理

    VideoWidget* m_vidWidget;     //视频通话窗口

	friend class  TalkWindowShell;
	
};

