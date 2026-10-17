#pragma once

#include <QSystemTrayIcon>
# include <QWidget>
#include "CustomMenu.h"

//系统托盘
class Systray  : public QSystemTrayIcon
{
	Q_OBJECT

public:
	Systray(QWidget *parent = nullptr);
	~Systray();

private :
	//系统托盘
	void initSystemTray();
	// 托盘菜单
	void addSystrayMenu();
public slots:
	void onIconActivated(QSystemTrayIcon::ActivationReason reason);
private :
	QWidget* m_parent; 
	CustomMenu* customMenu; 
};

