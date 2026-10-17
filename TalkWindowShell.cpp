#include "TalkWindowShell.h"
#include "CommonUtils.h"
# include "TalkWindow.h"
# include "EmotionWindow.h"
# include "TalkWindowItem.h"
#include "WindowManager.h"
#include "ReceiveFile.h"


# include <QListWidget>
# include <QSqlQueryModel>
# include <QMessageBox>
# include <QDir>


QString gfileName;	//文件名称
QString gfileData; //文件的数据

extern QString gLoginEmployeeID;


TalkWindowShell::TalkWindowShell(QWidget* parent )
	: BasicWindow(parent)
{
	ui.setupUi(this);
	initControl();
}

TalkWindowShell::~TalkWindowShell()
{
	delete m_enotionWindow;
	m_enotionWindow = nullptr; 
}

//添加聊天页面
void TalkWindowShell::addTalkWindow(TalkWindowItem * TalkWindowItem, const QString& uid)
{
	//ui.rightStackedWidget->addWidget(talkWindow);


	QListWidgetItem* aItem = new QListWidgetItem(ui.listWidget);
	m_talkWindowItemMap.insert(aItem, uid);

	aItem->setSelected(true);

	ui.listWidget->addItem(aItem);
	ui.listWidget->setItemWidget(aItem, TalkWindowItem);

	onTalkWindowItemClicked(aItem);

	connect(TalkWindowItem, &TalkWindowItem::signalCloseClicked, [=] (QString uid){
		//QString uid = m_talkWindowItemMap.value(aItem);
        m_TalkWindows->MessageSwitchFile(uid);
		QListWidgetItem* item = m_talkWindowItemMap.key(uid);
		m_talkWindowItemMap.remove(item);
		
		WindowManager::getInstance()->getWindowMap(uid);

		m_MemberName.remove(uid);
		m_GroupId.remove(uid);

		ui.listWidget->takeItem(ui.listWidget->row(aItem));
		delete TalkWindowItem;
		
		if (ui.rightStackedWidget->count() < 1) {
			close();
            ui.rightStackedWidget->removeWidget(m_TalkWindows);
		}

		});

}
//设置当前聊天部件
void TalkWindowShell::setCurrentWidget(QWidget* widget)
{

	ui.rightStackedWidget->setCurrentWidget(widget);

}


//添加聊天列表
void TalkWindowShell::addContactWidget(const QString& uid, 
										const QString& Name,
										const QString& Sign,
										const QString& Image,
										const QMap<QString ,QVariantMap>& params)
{	
	m_GroupId.insert(uid, params);
	m_MemberName.insert(uid, Name);
	//将当前已经打开的窗口添加进入映射 
	WindowManager::getInstance()->addWindowName(uid, params.value(uid));

    TalkWindowItem* talkwidowItem = new TalkWindowItem(m_TalkWindows, uid);
	QString URL = Image;	//拿取群聊头像数据
	talkwidowItem->setinitIndex(Name, URL);	//设置 Item项的头像名字

	QString strWindowName, strMsgLabel;
	strWindowName = Name;
	strMsgLabel = Sign;

    m_TalkWindows->setWindowName(strWindowName);//窗口的名称
	talkwidowItem->setMsgLabelContent(strMsgLabel);	 //左侧联系人文本
	addTalkWindow(talkwidowItem, uid);

}

const QMap<QListWidgetItem*, QString>& TalkWindowShell::getTalkWindowItemMap() const
{
	return m_talkWindowItemMap; 
}

//拿取 群成员数据
QMap<QString, QVariantMap> TalkWindowShell::getGroupMap(const QString& uid)
{
	return m_GroupId.value(uid);

}

//拿取列表成员Item项
QListWidgetItem* TalkWindowShell::getListWidgetItem(const QString& uid)
{
	return m_talkWindowItemMap.key(uid);
}

void TalkWindowShell::initControl()
{

	loadStyleSheet("TalkWindow");
	setWindowTitle(QString::fromUtf8("王老头"));

	m_enotionWindow = new EmotionWindow;
	m_enotionWindow->hide();

    m_TalkWindows = new TalkWindow(this);
	
    ui.rightStackedWidget->addWidget(m_TalkWindows);
    ui.rightStackedWidget->setCurrentWidget(m_TalkWindows);//绑定页面

	QList<int> leftWidgetSize; 
	leftWidgetSize << 154 << width() - 54; 
	ui.splitter->setSizes(leftWidgetSize);	//分类器设置尺寸
	
	//设置自定义风格
	ui.listWidget->setStyle(new CustomProxyStyle(this));

	connect(ui.listWidget, &QListWidget::itemClicked, this, &TalkWindowShell::onTalkWindowItemClicked);
	connect(m_enotionWindow, SIGNAL(signalEmotionItemClicked(int)), this, SLOT(onEmotionItemClicked(int)));
	connect(WindowManager::getInstance(), &WindowManager::onCutQListWidgetItem,
		this, &TalkWindowShell::onTalkWindowItemClicked);

    connect(m_enotionWindow, SIGNAL(signalEmotionWindowHide()),
            m_TalkWindows, SLOT(onSetEmotionBtnStatus()));
}


//表情按键点击事件
void TalkWindowShell::onEmotionBtnClicked(bool) {

	m_enotionWindow->setVisible(!m_enotionWindow->isVisible());
	//将控件得相对位置转化成屏幕得绝对位置
	QPoint emotionPoint = this->mapToGlobal(QPoint(0, 0));

	emotionPoint.setX(emotionPoint.x() + 170);
	emotionPoint.setY(emotionPoint.y() + 220);
	m_enotionWindow->move(emotionPoint);

}

//左侧列表点击后执行得槽函数
void TalkWindowShell::onTalkWindowItemClicked(QListWidgetItem* item) {

	QString uid = m_talkWindowItemMap.value(item);
	const QMap<QString, QVariantMap> params = m_GroupId.value(uid);
	const QString Name = m_MemberName.value(uid);

    //聊天页面修改
    m_TalkWindows->setWidgetUI(uid, params);
    m_TalkWindows->setWindowName(Name);//设置窗口名称
	//切换联系人视图
   m_TalkWindows->slotContactClicked(uid);

	//ui.rightStackedWidget->setCurrentWidget(talkWindowWidget);
}
//选择得表情
void TalkWindowShell::onEmotionItemClicked(int emotionNum) {

	TalkWindow* curTalkWinodw = dynamic_cast<TalkWindow*>(ui.rightStackedWidget->currentWidget());

	if (curTalkWinodw) {
		curTalkWinodw->addEmotionImage(emotionNum);
	}
}

