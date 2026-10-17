#pragma once

#include <QMenu>
# include <QMap>

//自定义菜单
class CustomMenu  : public QMenu
{
	Q_OBJECT

public:
	CustomMenu(QWidget *parent = nullptr);
	~CustomMenu();

public :
	//添加菜单
	void addCustomMenu(const QString& text, const QString& icon, const QString& name);
	// 获取动作
	QAction* getAction(const QString& text);
private:
	QMap<QString, QAction*>m_menuActionMap; 
};

