#ifndef CLIICKMENU_H
#define CLIICKMENU_H

#include <QWidget>
# include <QMenu>
# include <QAction>
# include <QTreeWidgetItem>
# include <QLineEdit>
# include <QWidgetAction>

class CliickMenu : public QMenu
{
    Q_OBJECT

public:
    explicit CliickMenu(QWidget *parent = nullptr);
    ~CliickMenu();

    void initContrt();
    void setCurrentAccount(const QString& Account);        //设置当前item的手机号
    void setTreeItem(QTreeWidgetItem* item);              //设置当前选中的item项
    void addGroupingAction(const QString& GroupingName); //添加联系人分组项
    QString getTreeItem();                              //返回当前选中的item项
    QTreeWidgetItem* getTreeGriupingItem();            // 返回选中的联系人项
    int getGroupingItemSize();                        //返回菜单添加了几个action
    QStringList getActionGroupingName();            //放回当前已经添加的联系分组名

signals:
    //添加到某个分组
    void addGroupingClicked(const QString& GroupingName);
    //添加联系人分组信号
    void addGrouping(const QString& GroupingName);
    //删除联系人的信号
    void DeleteContact();
private:
    QString m_Account;                      //当前点击的item项的手机号
    QTreeWidgetItem* m_item = nullptr;     //当前丢选中的item项
    QMenu *m_actMoveGroup;                //移动分组
    QWidgetAction *m_addGroup;           //添加分组
    QAction *m_DeleteUser;              //删除好友
    QLineEdit* m_Modification;         //添加联系人分组的名称
    QAction* m_addBtnGrouping;        //确定添加好友

};

#endif // CLIICKMENU_H
