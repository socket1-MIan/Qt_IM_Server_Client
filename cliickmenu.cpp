#include "cliickmenu.h"

CliickMenu::CliickMenu(QWidget *parent)
    : QMenu(parent)

{

    initContrt() ;
}

CliickMenu::~CliickMenu()
{
}

void CliickMenu::initContrt()
{

    m_actMoveGroup = addMenu("移动到分组");
    m_addGroup= new QWidgetAction(this);
    addSeparator();
    m_DeleteUser = new QAction("删除好友", this);

    m_Modification = new QLineEdit(this);
    m_Modification->setPlaceholderText("输入添加的分组名:");
    m_addGroup->setDefaultWidget(m_Modification);
    m_addBtnGrouping = new QAction("添加分组", this);

    addAction(m_addGroup);
    addAction(m_addBtnGrouping);
    addAction(m_DeleteUser);

    //点击添加联系人分组的信号
    connect(m_addBtnGrouping, &QAction::triggered, [=](){
        QString Name = m_Modification->text();
        if(!Name.isEmpty()){
            emit addGrouping(Name);
        }
        m_Modification->clear();
    });

    //点击删除好友时的信号
    connect(m_DeleteUser, &QAction::triggered, [=](){
        emit DeleteContact();
    });
}

//设置当前item的手机号
void CliickMenu::setCurrentAccount(const QString &Account)
{
    m_Account = Account;
}

//设置当前选中的item项
void CliickMenu::setTreeItem(QTreeWidgetItem *item)
{

    m_item = item;;
}

//添加联系人分组项
void CliickMenu::addGroupingAction(const QString &GroupingName)
{
    QAction* action = new QAction(GroupingName ,m_actMoveGroup);
    action->setData(GroupingName);
    m_actMoveGroup->addAction(action);
    connect(action, &QAction::triggered, [=](){

        emit addGroupingClicked(action->data().toString());
    });
}

//返回当前选中的item项
QString CliickMenu::getTreeItem()
{

    return m_Account;
}

// 返回选中的联系人项
QTreeWidgetItem *CliickMenu::getTreeGriupingItem()
{

    return m_item;
}

//返回菜单添加了几个action
int CliickMenu::getGroupingItemSize()
{
    return m_actMoveGroup->actions().size();

}

//放回当前已经添加的联系分组名
QStringList CliickMenu::getActionGroupingName()
{

    QStringList list;
    for(QAction* action : m_actMoveGroup->actions()){
        list.append(action->text());
    }

    return list;
}
