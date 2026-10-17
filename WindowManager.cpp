#include "WindowManager.h"
# include <QSqlQueryModel>
#include <QSqlDatabase>


//创建全局静态对象， 单列模式
Q_GLOBAL_STATIC(WindowManager, theInstance)

extern QString gLoginEmployeeID; //登录者的手机号码

WindowManager::WindowManager()
	:QObject(nullptr)
    ,m_talkwiddowshell(nullptr)

{
    connect(TcpClient::getInstance(),&TcpClient::ReceiverOfflineMSg,
            this ,&WindowManager::RecevierOfflineMsg);

}

WindowManager::~WindowManager()
{

}


//查找并返回窗口名称
QVariantMap WindowManager::findWidgetName(const QString & qsWindowName)
{
	if (m_windowMap.contains(qsWindowName)) {
		return m_windowMap.value(qsWindowName);
	}
	return QVariantMap();
}
//删除窗口
void WindowManager::deleteWindowName(const QString& qsWindowName)
{
	m_windowMap.remove(qsWindowName);
}
//添加新得聊天窗口
void WindowManager::addWindowName(const QString& qsWindowName, const QVariantMap& params)
{
	if (!m_windowMap.contains(qsWindowName)) {
		m_windowMap.insert(qsWindowName, params);
	}
}

WindowManager* WindowManager::getInstance()
{
	return theInstance;
}

//添加新的聊天列表
void WindowManager::addNewTalkWindow(const QString& uid,
	const QString& Name, 
	const QString& Sign, 
	const QString& image, 
	const QMap<QString, QVariantMap>& MapGroup)
{

    //是否与该好友存在离线消息
    if(m_unreadOfflineMap.contains(uid) || m_sessionUnreadMap.contains(uid)){
        //当前已经点击打开直接发送已读回执
        TcpClient::getInstance()->sendServerReanMsg(uid, gLoginEmployeeID);
        checkAndSendReadReceipt(uid);   //清空未读红点
    }

    if (m_talkwiddowshell == nullptr) {
        m_talkwiddowshell = new TalkWindowShell;
        // Shell 关闭销毁时，把指针置空，避免野指针
        connect(m_talkwiddowshell, &TalkWindowShell::destroyed, [=](QObject* obj) {
            Q_UNUSED(obj);
            m_talkwiddowshell = nullptr;
        });
    }

	if (!m_windowMap.contains(uid)) {	//判断当前这个ID号码是否已经创建

		m_strCreatingTalkId = uid;
		m_strCreatingTalkId = " ";
		//添加新的聊天列表
		m_talkwiddowshell->addContactWidget(uid, Name, Sign, image, MapGroup);
		//addWindowName(uid, MapGroup.value(uid));	//添加窗口 
	}
	else
	{
		//左侧聊天列表被选中
		QListWidgetItem* item = m_talkwiddowshell->getTalkWindowItemMap().key(uid);
        if(!item){
            return ;
        }
		item->setSelected(true);
		QMap<QString, QVariantMap> Map = m_talkwiddowshell->getGroupMap(uid);
		QListWidgetItem* Item = m_talkwiddowshell->getListWidgetItem(uid);
        if(!Item){
            return ;
        }
		emit onCutQListWidgetItem(Item);
	}

	m_talkwiddowshell->show();
	m_talkwiddowshell->activateWindow();

}





//拿取窗口与id的映射对象
void WindowManager::getWindowMap(const QString& uid)
{

	if (m_windowMap.contains(uid)) {
		m_windowMap.remove(uid);
	}
}

//获取正在创建的聊天窗口
QString WindowManager::getCreatingTalkId()
{
	return m_strCreatingTalkId;
}

//向服务器拉取离线消息数组
void WindowManager::getServerOffelineMsg()
{

    TcpClient::getInstance()->getOfflineMsg(gLoginEmployeeID);
}

//插入联系人手机号 ContactItem项
void WindowManager::InsertionContactItem(const QString &Account,  ContactItem* item)
{
    if(!m_peerToContactItem.contains(Account)){
        m_peerToContactItem.insert(Account, item);
    }

}

//外部调用添加未读消息计数
void WindowManager::updateSessionBadge(const QString &Account, int size)
{
    if(!m_sessionUnreadMap.contains(Account)){
        m_sessionUnreadMap.insert(Account, size);
        auto item =  m_peerToContactItem.value(Account);
        if(item){
            item->setUnreadCount(size);
        }
    }

    m_sessionUnreadMap[Account] += size;
    auto item =  m_peerToContactItem.value(Account);
    if(item){
        item->setUnreadCount(size);
    }
}

//外部调用清空与该用户的未读消息计数
void WindowManager::checkAndSendReadReceipt(const QString &Account)
{
    if(m_sessionUnreadMap.contains(Account)){
        m_sessionUnreadMap[Account] = 0;
        auto item =  m_peerToContactItem.value(Account);
        if(item){
            item->setUnreadCount(0);
        }

    }
}

//将接收到的离线消息数据追加到MSG中
void WindowManager::RecevierOfflineMsg(const QList<QVariantMap> &Msglist)
{
    m_unreadOfflineMap.clear();

    for(auto params : Msglist){
        ChatMsg msg;
        msg.send = params["sender"].toString();
        msg.receive = params["receiver"].toString();
        msg.msgType = params["msg_type"].toInt();
        msg.isSelf = false ;
        msg.content = params["msg_body"].toString();
        msg.timeStr = params["send_time"].toString();

        //添加离线消息
        if(!m_unreadOfflineMap.contains(params["sender"].toString())){
            m_unreadOfflineMap.insert(params["sender"].toString(), false);
        }
        //添加未读消息
        m_sessionUnreadMap[msg.send] += 1;

        //头像路径
        //先查看本地是否保存了当前头像缓存
        QString strURL = CCMianWidget::JudgmentContactImage(params["sender"].toString());
        if (!strURL.isEmpty()) {
            msg.avatarData = strURL;
        }
        else {
            //如果没有保存该文件路径缓存
            //使用默认得头像路径
            msg.avatarData = ":/Resources/MainWindow/girl.png";
        }

        //不是文本消息，通过服务器下发的图像名2次向服务器拉取这个图像2进制数据
        if(params["msg_type"].toInt() != 0){

            TcpClient::getInstance()->getServerOfflineMsgImaeg(params["receiver"].toString(),
                                                               params["msg_body"].toString());
            //先拿取图像路径，等服务器下发的图像在存入这个路径
            //如果下发速度慢于用户点击这个查看这个好友的离线消息，那么先用默认的图像渲染
            const QString Path = QPeratioFile::instance()->getJSONImageURL();
            msg.resPath = Path + "/" +params["msg_body"].toString(); //路径加服务器下发的图像名进行拼接
            //qDebug()<<"数据测试 当前给予的路径为： "<< msg.resPath;
        }

       ChatListModel::getInstance()->addRecvMsg(msg);
    }

    //拉取离线消息成功，向服务器发送回执
    TcpClient::getInstance()->sendServerOfflineMagMuccess(gLoginEmployeeID);

    auto item = m_peerToContactItem.constBegin();
    for(; item != m_peerToContactItem.constEnd(); item++){
        const QString name = item.key();
         ContactItem* ConItem = item.value();
        int size = m_sessionUnreadMap.value(name);
        if(!size) break;
        ConItem->setUnreadCount(size);

    }
}



