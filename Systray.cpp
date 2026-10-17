#include "Systray.h"


Systray::Systray(QWidget *parent):
	m_parent(parent),
	 QSystemTrayIcon(parent)
{
	
	initSystemTray(); 
	show(); 
}

Systray::~Systray()
{}
//系统托盘
void Systray::initSystemTray()
{
	customMenu = new CustomMenu(m_parent);
	this->setContextMenu(customMenu);
	this->setIcon(QIcon(":/Resources/MainWindow/app/logo.ico"));
	this->setToolTip(QStringLiteral("Qt-QQ"));
	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(onIconActivated(QSystemTrayIcon::ActivationReason)));
}

// 托盘菜单
void Systray::addSystrayMenu()
{
	
    customMenu->addCustomMenu("显示",
							":/Resources/MainWindow/app/logo.ico", 
							QStringLiteral("显示"));
    customMenu->addCustomMenu("退出",
					":/Resources/MainWindow/app/page_close_btn_hover.ico",
					QStringLiteral("退出"));

    connect(customMenu->getAction("显示"), SIGNAL(triggered(bool)),
			m_parent, SLOT(onShowNormal(bool)));
    connect(customMenu->getAction("退出"), SIGNAL(triggered(bool)),
		m_parent, SLOT(onShowQuit(bool)));
	
	customMenu->setFixedSize(160, 80);
	//进入时间循环
	//customMenu->move(QCursor::pos());
	customMenu->show();
	customMenu->exec(QCursor::pos());
	
}
void Systray::onIconActivated(QSystemTrayIcon::ActivationReason reason) {
	
	if (reason == QSystemTrayIcon::Trigger) {
		m_parent->show();
		m_parent->activateWindow(); 
	}
	else if (reason == QSystemTrayIcon::Context) {
		
		addSystrayMenu();
	}
}

