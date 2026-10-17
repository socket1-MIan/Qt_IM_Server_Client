#include "CustomMenu.h"
# include "CommonUtils.h"

CustomMenu::CustomMenu(QWidget *parent)
	: QMenu(parent)
{
	setAttribute(Qt::WA_TranslucentBackground);
	CommonUtils::loadStyleSheet(this, "Menu");
}

CustomMenu::~CustomMenu()
{}
//添加菜单
void CustomMenu::addCustomMenu(const QString & text, const QString & icon, const QString & name)
{
	QAction* pAction = new QAction(QIcon(icon), text);
	this->addAction(pAction);
	m_menuActionMap.insert(text, pAction);
	
}
// 获取动作
QAction* CustomMenu::getAction(const QString& text)
{
	return m_menuActionMap[text];
}

