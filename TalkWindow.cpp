#include "TalkWindow.h"
#include "TalkWindowShell.h"
# include "RootContatItem.h"
# include "ContactItem.h"
# include "CommonUtils.h" 
# include "WindowManager.h"
# include "TcpClient.h"
#include "qbuffer.h"

# include <QToolButton.h>
#include <QTreewidget.h>
# include <QPushButton>
# include <QToolTip>
# include <QMessageBox>
# include <QSqlQueryModel>
# include <QSqlQuery>
# include <QMessageBox>
# include <QSqlDatabase>


# include <QDateTime>

extern QString gLoginEmployeeID; //登录者的QQ号码

TalkWindow::TalkWindow(QWidget* parent)
	: QWidget(parent)
	//,m_groupType(grouType)
{
	// 获取全局默认Profile

	ui.setupUi(this);

	//initGroupTalkStatus(); //判断是否多人聊天 
	intControl();
	inintListView();	  //初始化视图模型
	
}

TalkWindow::~TalkWindow()
{
	WindowManager::getInstance()->deleteWindowName(m_talkId);
    delete m_vidWidget;

}



//修改窗口的UI来进行不同的联系人切换效果
void TalkWindow::setWidgetUI(const QString& uid,		
	const QMap<QString, QVariantMap>& ListGroup)
{
	if(!uid.isEmpty() && !ListGroup.isEmpty()){
	m_talkId = uid;				//切换联系人的ID

	//判断当前群号数据是否已经插入
	if (!m_MapGroupp.contains(uid))
	{
		QMap<QString, QVariantMap>::const_iterator i_r;
		i_r = ListGroup.constBegin();
		for (; i_r != ListGroup.constEnd(); i_r++) {
			QVariantMap params = i_r.value();
			QString Name = i_r.key();
			//在判断当前群号数据是否已经插入
			if(!m_MapGroupp.contains(Name))
				m_MapGroupp.insert(Name, params);
		}
	}

	//m_MapGroupp = ListGroup;	
	//m_MapGroupImage = MapGroupImage; 
	ui.treeWidget->clear();
	initGroupTalkStatus(); //判断是否多人聊天 
	if (m_isFroupTalk) {
		initTalkWindow_Server();	//多人聊天
	}
	else {
		initPtoPTiak();		//单聊
	}
	}
}

//将当前这个聊天页面写入本地文件缓存
void TalkWindow::MessageSwitchFile(const QString& uid)
{
	m_msgModel->SwitchFile(uid);
}

//添加表情图片
void TalkWindow::addEmotionImage(int emotionNum)
{
	ui.textEdit->setFocus(); 
	ui.textEdit->addEmtionUrl(emotionNum);

}
//设置窗口名称
void TalkWindow::setWindowName(const QString& name)
{
	ui.nameLabel->setText(name);

}

//获取TalkID
QString TalkWindow::getTalkID()
{
	return m_talkId;
}


//发送文件按键被点击了
void TalkWindow::onFileOpenBtnClicked(bool)
{

	QString Path = QFileDialog::getOpenFileName(this,
		"选择发送的图片",
		QDir::homePath(),
		"图片(*.png *.jpg)");
	if (Path.isEmpty()) {
		qDebug() << "发送的图片数据为空";
		return;
	}
	else {
		slotSendBtnClicked("", ChatMsg::MsgType::ImageMsg, Path, m_talkId);
	}

}
//判断是否为多人聊天
void TalkWindow::initGroupTalkStatus()
{
	
	// 2000 开头就是群号id
    if (m_talkId[0] == "1") //单独聊天
		m_isFroupTalk = false;
	else
		m_isFroupTalk = true; 
}

//获取公司部门的ID号
int TalkWindow::getCompDepID()
{
	QSqlDatabase sqlQueryBase = QSqlDatabase::database();

	if (!sqlQueryBase.open()) {
		QMessageBox::information(this, "提示", "在当前 TalkWndow 75行 数据库的连接失效");
	}

	QSqlQuery queryDepID(sqlQueryBase);
	queryDepID.prepare("SELECT departmentID FROM tab_department WHERE department_name = '公司群'");
	queryDepID.exec();
	queryDepID.next();
	int compyID = queryDepID.value(0).toInt();
	return compyID;
}


//初始化多人群聊
void TalkWindow::initTalkWindow_Server()
{
    m_isFroupTalk = false; //多人

	QTreeWidgetItem* pRootItem = new QTreeWidgetItem;
	//设置指示器得策略
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	//用于区分根项子项
	pRootItem->setData(0, Qt::UserRole, 0);
	RootContatItem* pItemName = new RootContatItem(false, ui.treeWidget);

	ui.treeWidget->setFixedHeight(646); // UIshell得高度减去UITalkwidgetde1头高

	//int GroupSize =  m_ListGorup.count();
	//设置右侧群聊成员列表的名称个 数量
	pItemName->setText(QString("成员列表" ));

	//插入分组节点
	ui.treeWidget->addTopLevelItem(pRootItem);
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);

	//展开
	pRootItem->setExpanded(true);
	//用户信息的列数
	//int columns = m_ListGorup[0].count() -1 ;

	QMap<QString, QVariantMap>::const_iterator ir;
	ir = m_MapGroupp.constBegin();
	for (; ir != m_MapGroupp.constEnd(); ir++) {
		
		 QVariantMap params = ir.value();

		QString Name = params["QQNember"].toString();				//名字
		QString employeeID = params["MobileNember"].toString();		//手机号
		QString GroupId = params["GroupId"].toString();			//获取当前的群号
		QString Sign = params["Sign"].toString();				//个性签名
		QString iamgeUrl = params["URL"].toString();//头像

		//单聊天对象是自己的时
		if (employeeID == gLoginEmployeeID) {
			//continue;
		}
		//判断当前的成员是否是否是这个群聊
		if (GroupId != m_talkId && m_talkId != "2000")
			continue; 

		//添加节点
		addPeopInfo(pRootItem, employeeID, Name, Sign, iamgeUrl);
	}
	
}

//单聊
void TalkWindow::initPtoPTiak()
{
    m_isFroupTalk = true; // 当前为单聊

	QPixmap pixSkin;
	pixSkin.load(":/Resources/MainWindow/skin.png");

	ui.widget->setFixedSize(pixSkin.size());

	QLabel* skinLabel = new QLabel(ui.widget);
	skinLabel->setPixmap(pixSkin);
	skinLabel->setFixedSize(ui.widget->size());

}


//添加窗口子项
void TalkWindow::addPeopInfo(QTreeWidgetItem* pRootGroupItem, 
	const QString& uId,
	const QString& Name,
	const QString& Sign,
	const QString& imageURL)
{
	QTreeWidgetItem* pChild = new QTreeWidgetItem();

	//添加子节点
	pChild->setData(0, Qt::UserRole, 1);
	pChild->setData(0, Qt::UserRole + 1, uId);
	ContactItem* pContactItem = new ContactItem(ui.treeWidget);

	QPixmap pix1;
	pix1.load(":/Resources/MainWindow/head_mask.png");


	QImage imageHeag(imageURL);
	if (imageHeag.isNull()) {
		qDebug() << "群成员列表头像数据载入失败";
		return;
	}


	static int i = 0;
	pContactItem->setHeadPixamp(CommonUtils::getRoundIamge(QPixmap::fromImage(imageHeag), pix1, pContactItem->getHeadLabelSize()));
	pContactItem->setUserName(Name);
	pContactItem->setSignName(Sign);

	pRootGroupItem->addChild(pChild);
	ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

	QString str = pContactItem->getUserName();
	m_groupPeopleMap.insert(pChild, str);

}

//添加名称
void TalkWindow::addPeopInfo(QTreeWidgetItem* pRootGroupItem, int employeeID)
{
	QTreeWidgetItem* pChild = new QTreeWidgetItem();
	
	//添加子节点
	pChild->setData(0, Qt::UserRole, 1);
	pChild->setData(0, Qt::UserRole + 1, employeeID);
	ContactItem* pContactItem = new ContactItem(ui.treeWidget);

	QPixmap pix1;
	pix1.load(":/Resources/MainWindow/head_mask.png");

	//获取名称，签名， 头像
	QString strName, strSign, strPicturePath;
	QSqlQueryModel queryInfoModel;
	queryInfoModel.setQuery(QString("SELECT employee_name,employee_sign,pictuie FROM tab_employess WHERE employeeID = %1")
		.arg(employeeID));

	QModelIndex nameIdex, signIndex, pictureIndex;
	nameIdex = queryInfoModel.index(0,0); //行 列
	signIndex = queryInfoModel.index(0, 1); //行 列
	pictureIndex = queryInfoModel.index(0, 2); //行 列

	strName = queryInfoModel.data(nameIdex).toString();
	strSign = queryInfoModel.data(signIndex).toString();
	strPicturePath = queryInfoModel.data(pictureIndex).toString();

	QImage imageHeag;
	imageHeag.load(strPicturePath);


	static int i = 0;
	pContactItem->setHeadPixamp(CommonUtils::getRoundIamge(QPixmap::fromImage(imageHeag), pix1, pContactItem->getHeadLabelSize()));
	pContactItem->setUserName(strName);
	pContactItem->setSignName(strSign);

	pRootGroupItem->addChild(pChild);
	ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

	QString str = pContactItem->getUserName();
	m_groupPeopleMap.insert(pChild, str);
}

// 通过点击群成员来创建左侧联系人窗口
void TalkWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{

	
	bool bIsChlid = item->data(0, Qt::UserRole).toBool();
	if (bIsChlid) {
		const QString& uid = item->data(0, Qt::UserRole + 1).toString();
		//const QStringList& list = m_MapGroupp.value(uid);
		QVariantMap params = m_MapGroupp.value(uid);
		QString Name = params["QQNember"].toString();//	名字
		QString employeeID = params["MobileNember"].toString();//手机号
		QString Sign = params["Sign"].toString();	//个性签名
		QString image = params["URL"].toString();//头像

		QMap<QString, QVariantMap>Map;
		Map.insert(uid, params);


		QString strPeopleName = m_groupPeopleMap.value(item);
		WindowManager::getInstance()->addNewTalkWindow(uid,Name, Sign, image, Map);

	}


}

//关闭按键点击
void TalkWindow::on_closeBtn_clicked()
{
	
}

//切换联系人视图
void TalkWindow::slotContactClicked(const QString& contactId)
{
	// 模型切换数据源，视图自动刷新
	m_msgModel->switchContact(contactId);

	//滚动到底部
	m_listview->scrollToBottom();
}

//接收到对方消息时
void TalkWindow::addRecvMsg(const ChatMsg& Message)
{

    ChatMsg msg =  Message;
    QString Name = msg.send;	//发送这条消息的对象

    //自动存入对应联系人缓存,如果是当前聊天的活动窗口则立刻刷新UI
    m_msgModel->appendContactMsg(Name, msg);
    if (m_msgModel->rowCount() > 0)	//判断当前是否 有消息数据避免强行滚动到底部
        m_listview->scrollToBottom();//滚动到底部

}

//我发送消息
void TalkWindow::slotSendBtnClicked(const QString& Naber,
									const ChatMsg::MsgType& msgType,
									const QString& path,
									const QString& uid)
{

	//m_msgModel->switchContact(m_talkId);
    QPixmap pixmmap;
    ChatMsg msg;
	QString URL = m_MapGroupp.value(gLoginEmployeeID)["URL"].toString();
	pixmmap.load(URL);
	if (pixmmap.isNull()) {
		qDebug() << "视图模型中头像数据加载失败"; 
		}

	msg.userName = gLoginEmployeeID;	//发送者的名字
	msg.content = Naber;				//发送的文本 
	msg.timeStr = QDateTime::currentDateTime().toString("HH:mm");//时间戳
    msg.avatarData =  URL;				//载入自己的头像图像数据
	msg.isSelf = true;					//自己发送的消息体
	msg.msgType = msgType;				//消息体类型
    msg.resPath = path;					//发送的图像数据缓存路径(因为在发的时候也需要在我的界面渲染先用路径)
	msg.receive = uid;					//接收者对象
	msg.send = gLoginEmployeeID;		//发送者的手机号码

    // 拿到当前联系人ID追加消息
	//QString curId = m_msgModel->getCurrentContactId();
	m_msgModel->appendContactMsg(m_talkId, msg);
	m_listview->scrollToBottom();
	//m_listview->clear();
    //进行消息转发
    if(uid[0] == "2"){
        //群聊消息
        TcpClient::getInstance()->SendGroupId(msg);
    }else if(uid[0] == "1"){
        //向联系人发送指定消息
        TcpClient::getInstance()->SendUser(msg);
    }
}

//初始化视图模型代理
void TalkWindow::inintListView()
{

	m_listview = new QListView;
	// 去除分割线、背景、边框
	m_listview->setFrameShape(QFrame::NoFrame);
	m_listview->setSpacing(4);
	m_listview->setVerticalScrollMode(QListView::ScrollPerPixel);

    // ✅ 改用全局单例，所有TalkWindow共享同一个模型
    m_msgModel = ChatListModel::getInstance();
    // ⚠️ 注意：单例的parent不能是ui.MsgWidget，否则窗口关闭时会被一起delete
    // 单例应该在程序启动时创建一次，生命周期和程序一致

	m_delegate = new ChatMsgDelegate(ui.MsgWidget);
	m_listview->setModel(m_msgModel);
	m_listview->setItemDelegate(m_delegate);

	QVBoxLayout* VB = new QVBoxLayout(ui.MsgWidget);
	VB->setSpacing(6);
	VB->setContentsMargins(6, 6, 6, 6);
	VB->addWidget(m_listview);

}

//进行通话请求
void TalkWindow::RequestVide()
{
    initGroupTalkStatus();
    if(! m_isFroupTalk){
        qDebug()<< "----- 点击视频通话 ----" ;
        TcpClient::getInstance()->slotCall(m_talkId);
    }


}



void TalkWindow::intControl()
{
	QList<int> rightWidgetSize;
	rightWidgetSize << 600 << 138;
	ui.bodySplitter->setSizes(rightWidgetSize);

	ui.textEdit->setFontPointSize(10);
	ui.textEdit->setFocus();

	connect(ui.sysmin, SIGNAL(clicked(bool)), parent(), SLOT(onShowMin(bool)));
	connect(ui.sysclose, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	connect(ui.closeBtn, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	
	connect(ui.faceBtn, SIGNAL(clicked(bool)), parent(), SLOT(onEmotionBtnClicked(bool)));
	connect(ui.sendBtn, SIGNAL(clicked(bool)), this, SLOT(onSendBtnClicked(bool)));

	connect(ui.fileopenBtn, SIGNAL(clicked(bool)), this, SLOT(onFileOpenBtnClicked(bool)));

	//connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.treeWidget, &QTreeWidget::itemDoubleClicked, this, &TalkWindow::onItemDoubleClicked);
	
	//接收来自服务器返回的群聊消息数
	connect(TcpClient::getInstance(), &TcpClient::QueryGroupMessage, this, &TalkWindow::addRecvMsg);

    //初始化视频通话窗口
    m_vidWidget = new VideoWidget;

    TcpClient::getInstance()->initVideWidget(m_vidWidget->getBtnWidget(), m_vidWidget->getVBoxLayout());

    connect(TcpClient::getInstance(), &TcpClient::sinVideShow,
        m_vidWidget, &VideoWidget::on_show);
    connect(TcpClient::getInstance(), &TcpClient::sinVideHide,
           m_vidWidget, &VideoWidget::on_hide);
    //挂断视频通话
    connect(m_vidWidget->getStopBtn(), &QPushButton::clicked,
            TcpClient::getInstance(), &TcpClient::sotpCall);

    //绑定视频通话按键
    connect(ui.videBtn, &QPushButton::clicked,
            this, &TalkWindow::RequestVide);

}

//发送信号
void TalkWindow::onSendBtnClicked(bool) {

    QVariantMap params;
    params["cmd"] = 101 ;
    params["fromAccount"] = "User1";

    //TcpClient::getInstance()->slotCall();

	if (ui.textEdit->toPlainText().isEmpty()) {
		QToolTip::showText(this->mapToGlobal(QPoint(630, 660)),
		QString::fromUtf8("发送的信息不能为空"), this,
		QRect(0,0, 120, 100), 2000);
		return;
	}
	else
	{
		const QString& strData = ui.textEdit->toPlainText().trimmed();
		if (strData.isEmpty()) return;

		slotSendBtnClicked(strData, ChatMsg::MsgType::TextMsg, "", m_talkId);
	}
	
	
}

