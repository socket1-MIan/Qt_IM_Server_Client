#pragma once

#include <QObject>
# include "TalkWindowShell.h"
# include "TalkWindowItem.h"
# include "TalkWindow.h"
# include "TcpClient.h"
# include "ChatMsg.h"

#include "ChatListModel.h"

class WindowManager  : public QObject
{
	Q_OBJECT

public:
	WindowManager();
    ~WindowManager();
public:
	//查找并返回窗口名称
	QVariantMap findWidgetName(const QString& qsWindowName);
	//删除窗口
	void deleteWindowName(const QString& qsWindowName);
	//添加新得聊天窗口
	void addWindowName(const QString& qsWindowName, const QVariantMap& params);

	static WindowManager* getInstance();
	//添加新的聊天列表
	void addNewTalkWindow(const QString& uid, const QString& Name, 
		const QString& Sign,
		const QString& image, 
		const QMap<QString, QVariantMap>& MapGroup);

    void getWindowMap(const QString& uid);         //拿取窗口与id的映射对象
    QString getCreatingTalkId();                  //获取正在创建的聊天窗口
    void getServerOffelineMsg();                 //向服务器拉取离线消息数组

    //插入联系人手机号 ContactItem项
    void InsertionContactItem(const QString& Account,  ContactItem* item);
    //外部调用添加未读消息计数
    void updateSessionBadge(const QString&Account, int size);
    //外部调用清空与该用户的未读消息计数
    void checkAndSendReadReceipt(const QString& Account);

signals:
	void onCutQListWidgetItem(QListWidgetItem*);	//切换列表itme

private slots:
    //将接收到的离线消息数据追加到MSG中
    void RecevierOfflineMsg(const QList<QVariantMap> &Msglist);

private:
    TalkWindowShell* m_talkwiddowshell;
	QMap<QString, QVariantMap> m_windowMap;		//ID号与群号(联系人)信息的映射
	QString m_strCreatingTalkId = "";			//正在创建的聊天窗口(QQ号)
    QMap<QString, bool> m_unreadOfflineMap; //key=发送方账号，true：存在未发送回执的离线消息
    QMap<QString, int> m_sessionUnreadMap;  //全部的未读消息计数 key = 对方手机号 value = 未读的消息数量
    QMap<QString, ContactItem*> m_peerToContactItem; //key = 用户账号 value = m_peerToContactItemk
};

