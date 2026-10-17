#pragma once

#include <QtWidgets/QWidget>
#include "ui_CCMianWidget.h"

# include "basicwindow.h"
# include "SkinWindow.h"
#include "Systray.h"
# include "GroupMember.h"
#include "friends.h"
#include "cliickmenu.h"

# include <QProxyStyle>
# include <QPainter>
# include <QTimer>
# include <QResizeEvent>
# include <QEvent>
# include <QTreeWidgetItem>
# include <QMouseEvent>
# include <QMenu>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


class CCMianWidget : public BasicWindow
{
    Q_OBJECT

public:
    CCMianWidget(QString account,bool isAccountLogin,QWidget *parent = nullptr);
    ~CCMianWidget();
    
    //初始化计时器
    void initTimer(); 
    //初始化部件
    void initControl(); 
    // 初始化用户名，等级头像
    void setUserName(const QString& username);      //用户名
    void setLevelPixmap(int level);                 //等级
    void setHeadPixmap(const QString& headPath);    //头像
    void setStatusMenuIcon(const QString& statusPath); //在线状态

    // 添加应用部件
    QWidget* addOtherAppExtension(const QString& apPath,const QString& appName);
    void updateSeachStyle();    // 更新搜索样式
    void initContactTree();     //初始化联系人

    //初始化联系人分组根节点
    void initRootNodeContact(QTreeWidgetItem* NodeTree, const QString& strName);
    void addContactTree(const QString& strName);     //添加联系人树
   
    //将头像数据写入文件
    void WriteGroupImageFile(const QString& GroupId,const QByteArray& ImagePkg);  
    void WriteImageFile(const QMap<QString, QByteArray>&);  //将服务返还的图像数据写入进文件
    void WriteGroupFile(const QList<QStringList>&);        //将群成员对象写入进JSON文档
    //将群的基本信息写入JSON文件  
    bool WriteGroupInformationFile(const QMap<QString, QVariantMap>&);

    void TakeoutGroupFile(const QMap<QString,  QList<GroupMenber>>&); //拿取群成员的缓存数据
    void TakeoutImageFile();    //拿取群成员图像缓存数据(不包含群头像)

    //拿取群信息数据(所有加入的群聊和添加的联系人列表)
    void TakeoutGroupIdFile(const QString&,QMap<QString, QVariantMap>&);
    void TakeoutFile();     //拿取缓存数据
    bool JudgmentFile();    //判断成员缓存数据是否已经存在
    void LoadingFile();     //加载缓存数据,添加到UI界面
    void WriteContactGrouping(const QMap<QString, QList<QString>>&);    //将联系人分组项写入到JSON文件
    void TakeitContactGrouping(const QMap<QString, QList<QString>>&);    //从JSON拿取联系人分组缓存

    QByteArray pixmapToBytes(const QPixmap& pix);   //将图像数据转换为二进制数据

   static QString JudgmentContactImage(const QString& uid);   //查询并返回头像路径
private :
    //尺寸事件
    void resizeEvent(QResizeEvent* event) override;

    //键盘事件
    bool eventFilter(QObject* obj, QEvent* event)override;
    void mousePressEvent(QMouseEvent* event)override;

private slots:
    QString getHeadPicturePath();                       //获取登录者的头像

    //添加联系人到树图根项
    void addTreeItem(const QString& strNamem,
                const QString& strSign,
                const QString& Account,
                const QImage& image,
                 QTreeWidgetItem* item);
    //删除联系人分组下面的子项
    void DeleteTreeItem();
    //将添加的联系人和群聊的分组关系写入到缓存文件
    void WriteGroupingFile();
    //读取缓存文件中的联系人分组
    void ReadingGrouping();

private slots:
   
    //app 的公能实现接口
    void onAppIconClicked();
    void onItemClicked(QTreeWidgetItem* itme, int column); //点击
    void onItemExpanded(QTreeWidgetItem* item ); //展开
    void onItemCollapsed(QTreeWidgetItem* item); //收缩
    void onItmeDoubleClicked(QTreeWidgetItem* item, int column); //双击

    //接收服务端返回的群聊成员数据
    void onGetServerGoup(QList<QStringList>list, int rows, int colunms, const QMap<QString, QByteArray>& map);
    //拿取服务器返回的账号数据
    void getServerAccount(QMap<QString, QString>map, QString strGroupId, QString strSign);

    //接收来自服务器的联系人列表包含(加入的群聊)
    void getServerContact(QString strData, const QVariantMap params);
    //接收来自服务器的用户信息数据
    void getServerUserInformation(const bool& isEmpty,
                                  const QString& strName,
                                  const QByteArray& ImageData,
                                  const QString& strSign);
    //接收来自服务器的添加好友的信息
    void addServerFriendsInformation(const QString& strNamem,
                                      const QString& strSign,
                                      const QString& Account,
                                      const QByteArray& imageData);
    //接收有用户请求添加自己为好友得请求
    void addUserServer(const QString& strNamem,
                       const QString& strSign,
                       const QString& Account,
                       const QByteArray& imageData);


    //右键菜单弹出
    void ClickedTreGrouping(const QPoint& pos);

    //将选中的联系人移动到其他分组
    void MoveGroping(const QString& GroupingName);

signals:
    
    void GroupDataCompleted(); //当前所有群聊已经加载完成
    //返回从缓存数据中提前到的群聊信息数据
    void QueryServerGroupID(QString sign, QImage image, QString GroupName, QString GroupID);
    // 发送当前所有联系人缓存数据拿取完成，通知加载联系人分组项目
    void NotificationsGroupingLoafingUI();

private:
    Ui::CCMianWidgetClass ui;

    QTreeWidgetItem* m_RootNodeGroupTreeItem;     //群聊联系人分组根节点
    QTreeWidgetItem* m_RootNodeContactTreeIteml; //联系人联系人分组根节点

    //QMap<QTreeWidgetItem*, QString> m_groupMap; //所有分组得分组项
    bool m_isAccounyLogin;      //当前是否为账号登录
    QString m_accout;           //记录登录时的账号或QQ号

    QMap<QString, QString> m_Account;   //记录当前用户的账号数据
    QStringList m_groupId;              //当前客户端所在的群聊号
    int m_GroupSize = 0;                //记录当前拿取到几个群聊数据

    QImage m_UserImage;     //记录当前拿取到的头像2进制数据
    QString m_GrouId;       //记录当前的群号Id
    QString m_Sign;         //记录当前的签名
    QString m_Text;         //记录输入的添加好友的手机号

    Friends* FriendsDialog = nullptr; //添加好友
    Friends* FriendsAgree = nullptr; //是否同意添加好友

    QMap<QString, QVariantMap> m_Group;         //记录的所有加入的群聊成员数据
    QMap<QString, QVariantMap> m_Contact;       //记录当前的聊天列表(包含加入的群聊添加的聊天对象)
    static QMap<QString, QString> m_ContactImg; //当前群聊成员手机号与图像数据的映射
    QMap<QString, QList<QString>> m_TreeItem;   //联系人列表与联系人分组节点映射
    QMap<QString, QTreeWidgetItem*> m_TreeGroupingItem; //分组名与联系人分组item的映射

    //QMap<QString, QString> m_ContactImgURLl;//当前群聊成员手机号与图像路径的映射
    CliickMenu *m_Menu;     //右键菜单项
};

