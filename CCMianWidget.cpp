#include "CCMianWidget.h"
#include "RootContatItem.h"
#include "ContactItem.h"
# include "WindowManager.h"
# include "TalkWindowShell.h"
#include "TcpClient.h"
#include "QPeratioFile.h"

# include <QApplication>
# include <QSqlQuery>
# include <QSqlDatabase>
# include <QMessageBox>
# include <QBuffer>

QString gstrLoginHeafPath;  //登录的头像
extern QString gLoginEmployeeID; //登录者的手机号码
QMap<QString, QString> CCMianWidget::m_ContactImg;  //全局初始化静态对象

class CustomProxyStyle : public QProxyStyle {

public :
    virtual void drawPrimitive(PrimitiveElement element, QStyleOption* option,
        QPainter* painter, const QWidget* widget = nullptr) const {

        if (element == PE_FrameFocusRect) {
            return;
        }
        else {
            QProxyStyle::drawPrimitive(element, option, painter, widget);

        }
    }

};

CCMianWidget::CCMianWidget(QString account, bool isAccountLogin,QWidget *parent)
    : BasicWindow(parent)
    ,m_isAccounyLogin(isAccountLogin)
    ,m_accout(account)
{
    ui.setupUi(this);
    //m_ui.setupUi(this);

    setWindowFlags(windowFlags() | Qt::Tool);
    loadStyleSheet("CCMainWindow");
    //setHeadPixmap(getHeadPicturePath());
    initControl();
    initTimer();



}

CCMianWidget::~CCMianWidget()
{

    //关闭时写入联系人分组缓存
    WriteGroupingFile();
}
//初始化计时器
void CCMianWidget::initTimer()
{
    QTimer* timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer, &QTimer::timeout, [=]() {
        static int level = 0;
        if (level == 99) {
            level = 0;
        }
        level++;
        setLevelPixmap(level);

        });
    timer->start(); 
}
//初始化部件
void CCMianWidget::initControl()
{
    // 树获取焦点时不绘制边框
    ui.treeWidget->setStyle(new CustomProxyStyle);
    setLevelPixmap(0);
    //setHeadPixmap(":/Resources/MainWindow/girl.png");
    setStatusMenuIcon(":/Resources/MainWindow/StatusSucceeded.png");

    //初始化 app 的部件
    QHBoxLayout* appupLayout = new QHBoxLayout;
    appupLayout->setContentsMargins(0, 0, 0, 0);
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_7.png", "app_7"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_2.png", "app_2"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_3.png", "app_3"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_4.png", "app_4"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_5.png", "app_5"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_6.png", "app_6"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/skin.png", "app_skin"));
    appupLayout->setSpacing(2);
    appupLayout->addStretch(); 
    ui.appWidget->setLayout(appupLayout);
    
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_10.png", "app_10"));
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_8.png", "app_8"));
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_11.png", "app_11"));
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_9.png", "app_9"));
    ui.bottomLayout_up->addStretch(); 

    //初始化联系人 
    initContactTree();

    //个性签名
    ui.lineEdit ->installEventFilter(this);
    //好友搜索
    ui.searchLineEdit->installEventFilter(this);

    //接收退出应用程序的信号
    connect(this, &CCMianWidget::execProgram,
            this, &CCMianWidget::WriteGroupingFile);

    connect(ui.sysmin, &QPushButton::clicked, this,
       &CCMianWidget::onShowHide);
    connect(ui.sysclose, SIGNAL(clicked(bool)), this, SLOT(onShowClose(bool)));

    //接收服务端发回的用户账号数据
    connect(TcpClient::getInstance(), &TcpClient::QUeryServerAccount, this, &CCMianWidget::getServerAccount);

    //初始化用户头像
    //接收服务端返回的头像数据的信号
    connect(TcpClient::getInstance(), &TcpClient::QueryServerImage, [=](QImage image) {
        m_UserImage = image;

        QPixmap pix;
        pix.load(":/Resources/MainWindow/head_mask.png");
        ui.headLabel->setPixmap(getRoundImage(QPixmap::fromImage(image), pix, ui.headLabel->size()));
        });
   
    //接收服务端成功返回的群聊成员信息信号
    connect(TcpClient::getInstance(), &TcpClient::QueryServerGroup,
        this, &CCMianWidget::onGetServerGoup);


    //拿取左侧消息列表
    //拿取加入的群聊
    connect(TcpClient::getInstance(), &TcpClient::QueryServerContact,
        this, &CCMianWidget::getServerContact);

    Systray* systray = new Systray(this);
    //添加好友窗口
    FriendsDialog = new Friends(this);
    FriendsDialog->hide();
    //发送添加好友
    connect(FriendsDialog->getAddBtn(), &QPushButton::clicked, [=](){

        TcpClient::getInstance()->addServerUserFriends(m_Text, gLoginEmployeeID);
        FriendsDialog->hide();
    });


    //是否同意添加好友窗口
    FriendsAgree = new Friends(this);
    FriendsAgree->getAddBtn()->setText("同意");
    FriendsAgree->hide();

    //当有用户请求添加我为好友时显示该窗口
    connect(TcpClient::getInstance(), &TcpClient::QueryAddUser,
            this, &CCMianWidget::addUserServer);

    //点击同意添加好友得申请
    connect(FriendsAgree->getAddBtn(), &QPushButton::clicked, [=](){
        const QString addAccount =  FriendsAgree->getBelAccount()->text();
        const QString addNambe = FriendsAgree->getBelName()->text();
        const QPixmap pixmap = FriendsAgree->getLabImage();
        const QString addSign = FriendsAgree->getLabSignStr();

        QByteArray pkg = pixmapToBytes(pixmap);
        if(pkg.isEmpty()){
            qDebug()<< "同意添加好友得申请并添加到好友分组时，头像转2进制失败";
            AdministratorWidget::getInstance()->appendLog("同意添加好友得申请并添加到好友分组时，头像转2进制失败");
            return ;
        }

        addServerFriendsInformation(addNambe, addSign, addAccount, pkg);

        TcpClient::getInstance()->addArreeAddUser(gLoginEmployeeID, addAccount);
    });

    //将好友添加界面与用户数据返回信号绑定
    connect(TcpClient::getInstance(), &TcpClient::QUeryUserInformation,
            this, &CCMianWidget::getServerUserInformation);

    //接收返回的同意好友添加申请的信息
    connect(TcpClient::getInstance(), &TcpClient::QueryUserFriendsInformation,
            this, &CCMianWidget::addServerFriendsInformation);

    ui.searchBtn->installEventFilter(this);

    //设置右键菜单
    m_Menu = new CliickMenu(this);
    ui.treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    //右键点击弹出菜单
    connect(ui.treeWidget, &QTreeWidget::customContextMenuRequested,
            this, &CCMianWidget::ClickedTreGrouping);
    //接收来自菜单项的添加到联系人分组信号
    connect(m_Menu, &CliickMenu::addGroupingClicked,
            this, &CCMianWidget::MoveGroping);
    //接收添加联系人分组的信号
    connect(m_Menu, &CliickMenu::addGrouping,
            this, &CCMianWidget::addContactTree);
    //接收删除联系人的信号
    connect(m_Menu, &CliickMenu::DeleteContact,
            this, &CCMianWidget::DeleteTreeItem);

    WindowManager::getInstance();

}
// 初始化用户名
void CCMianWidget::setUserName(const QString& username)
{

    //根据内容调整自己得尺寸
    ui.nameLabel->adjustSize(); 
    //文本过长则进行省略..
    //fontMetrics()返回QFontMetrics类对象
    QString name =  ui.nameLabel->fontMetrics().elidedText(username, Qt::ElideRight,
        ui.nameLabel->width());

    ui.nameLabel->setText(name);
}
// 初始化等级
void CCMianWidget::setLevelPixmap(int level)
{
    QPixmap levelPixmap(ui.levelBtn->size());
    levelPixmap.fill(Qt::transparent);

    QPainter painter(&levelPixmap);
    painter.drawPixmap(0, 10, QPixmap(":/Resources/MainWindow/lv.png"));
   
    int unitNum = level % 10; // 个位数
    int tenNum = level / 10; //十位数

    //十位   截取图片中的部分进行绘制
    // drawPixmap（绘制点 x, 绘制点 y,） 指定绘制图片的位置
    painter.drawPixmap(10, 10, QPixmap(":/Resources/MainWindow/levelvalue.png"), tenNum * 6, 0, 6, 7);
    
    //个位
    painter.drawPixmap(16,10, QPixmap(":/Resources/MainWindow/levelvalue.png"), unitNum * 6, 0, 6, 7);

    ui.levelBtn->setIcon(levelPixmap);
    ui.levelBtn->setIconSize(ui.levelBtn->size());

}
// 初始化头像
void CCMianWidget::setHeadPixmap(const QString& headPath)
{
   
    QPixmap pix; 
    pix.load(":/Resources/MainWindow/head_mask.png");
    ui.headLabel->setPixmap(getRoundImage(QPixmap(headPath), pix, ui.headLabel->size()));


}

//在线状态
void CCMianWidget::setStatusMenuIcon(const QString& statusPath)
{
   
    QPixmap statusBtnPixmap(QSize(16, 16));
    statusBtnPixmap.fill(Qt::transparent);

    QPainter painter(&statusBtnPixmap);
    painter.drawPixmap(0, 0, QPixmap(statusPath));

    ui.stausBtn->setIcon(statusBtnPixmap);
    ui.stausBtn->setIconSize(ui.stausBtn->size());

}
// 添加应用部件
QWidget* CCMianWidget::addOtherAppExtension(const QString& apPath,const QString& appName)
{
    QPushButton* btn = new QPushButton(this);
    btn->setFixedSize(20, 20);

    QPixmap pixmap(btn->size());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    QPixmap appPixmap(apPath);
    painter.drawPixmap((btn->width() - appPixmap.width()) / 2,
            (btn->height() - appPixmap.height()) /2, appPixmap);
    btn->setIcon(pixmap);
    btn->setIconSize(btn->size());
    btn->setObjectName(appName);
    btn->setProperty("hasborder", true);

    connect(btn, &QPushButton::clicked, this, &CCMianWidget::onAppIconClicked);

    return btn;
}
// 更新搜索样式
void CCMianWidget::updateSeachStyle()
{

    ui.searchLineEdit->setStyleSheet("QLineEdit#searchLineEdit{background-color:rgba(245,245,245,100)}");
    //                                .arg(m_colorBackGround.red())
    //                               .arg(m_colorBackGround.green())
    //                                .arg(m_colorBackGround.blue()));
    ui.searchLineEdit->clear();
}

//初始化联系人
void CCMianWidget::initContactTree()
{

    //展开与收缩时得信号
    //点击信号
    connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
        this, SLOT(onItemClicked(QTreeWidgetItem*, int)));
    //展开信号
    connect(ui.treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), 
        this, SLOT(onItemExpanded(QTreeWidgetItem*)));
    //收缩信号
    connect(ui.treeWidget, &QTreeWidget::itemCollapsed,
        this, & CCMianWidget::onItemCollapsed);
    //双击信号
    connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
        this, SLOT(onItmeDoubleClicked(QTreeWidgetItem*, int)));


    //初始化群组联系人分组根节点
    m_RootNodeGroupTreeItem = new QTreeWidgetItem(ui.treeWidget);
    //初始化联系人联系人分组根节点
    m_RootNodeContactTreeIteml = new QTreeWidgetItem(ui.treeWidget);

    //添加联系人分组根节点
    initRootNodeContact(m_RootNodeGroupTreeItem, "群聊");
    initRootNodeContact(m_RootNodeContactTreeIteml, "联系人");

    //接收来自服务器的群聊信息数据，并添加到联系人树中
    connect(TcpClient::getInstance(), &TcpClient::QueryServerGroupID, [=](const QString& strNamem,
                                                                          const QString& strSign,
                                                                          const QString& Account,
                                                                          const QImage& image){

        //接收到的群聊消息数据默认存储到群聊，联系人分组
        addTreeItem(strNamem, strSign, Account, image, m_RootNodeGroupTreeItem);
    });
    //当联系人缓存数据拿取成功，则开始加载联系人分组缓存
    connect(this, &CCMianWidget::NotificationsGroupingLoafingUI,
                this, &CCMianWidget::ReadingGrouping);
}

//初始化联系人分组根节点
void CCMianWidget::initRootNodeContact(QTreeWidgetItem *NodeTree, const QString& strName)
{
    if(!m_TreeGroupingItem.contains(strName)){

    NodeTree->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    NodeTree->setData(0, Qt::UserRole, 0);
    NodeTree->setData(0,Qt::UserRole + 1, strName);
    RootContatItem* pItemName = new RootContatItem(true, ui.treeWidget);
    pItemName->setText(strName);
    ui.treeWidget->addTopLevelItem(NodeTree);
    ui.treeWidget->setItemWidget(NodeTree, 0, pItemName);

    //插入联系人的树节点
   // QList<QString> list;
    //m_TreeItem.insert(strName, list);
    //插入联系人名称与分组item
    m_TreeGroupingItem.insert(strName, NodeTree);
    }
    else{
        qDebug()<<"添加当前联系人分组已经重复添加";
    }
}

//添加联系人树
void CCMianWidget::addContactTree(const QString &strName)
{

    QTreeWidgetItem* m_pRootGroupItem = new QTreeWidgetItem;

    m_pRootGroupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    m_pRootGroupItem->setData(0, Qt::UserRole, 0);
    m_pRootGroupItem->setData(0,Qt::UserRole + 1, strName);
    RootContatItem* pItemName = new RootContatItem(true, ui.treeWidget);
    pItemName->setText(strName);
    ui.treeWidget->addTopLevelItem(m_pRootGroupItem);
    ui.treeWidget->setItemWidget(m_pRootGroupItem, 0, pItemName);

    //插入联系人的树节点
    //QList<QString> list;
    //m_TreeItem.insert(strName, list);
    //插入联系人名称与分组item
    m_TreeGroupingItem.insert(strName, m_pRootGroupItem);
}

//添加联系人到树图根项
void CCMianWidget::addTreeItem(const QString &strNamem,
                               const QString &strSign,
                               const QString &Account,
                               const QImage &image,
                                QTreeWidgetItem *item)
{
    if(item){

    //初始添加到根项
    QTreeWidgetItem* pChild = new QTreeWidgetItem;
    QPixmap pix;
    pix.load(":/Resources/MainWindow/head_mask.png");

    pChild->setData(0, Qt::UserRole, 1);
    pChild->setData(0, Qt::UserRole + 1, Account);

    ContactItem* pContactItem = new ContactItem(ui.treeWidget);
    pContactItem->setHeadPixamp(getRoundImage(QPixmap::fromImage(image), pix, pContactItem->getHeadLabelSize()));
    pContactItem->setUserName(Account);

    //将pContactItem 插入到QMap中保管
    WindowManager::getInstance()->InsertionContactItem(Account, pContactItem);

    item->addChild(pChild);
    ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

    }
    else{
        //qDebug()<< "数据测试 --- 当场item为初始化";
    }
}

//删除联系人分组下面的子项
void CCMianWidget::DeleteTreeItem()
{

    QTreeWidgetItem* item = m_Menu->getTreeGriupingItem();
    QTreeWidgetItem* tree_item = item->parent();    //返回父节点
    if(tree_item){
        tree_item->removeChild(item); // 从父节点移除
        QString StrIndex =  m_Menu->getTreeItem();
        m_Contact.remove(StrIndex);
        QList<QString>& list = m_TreeItem[StrIndex];    //拿到地址的引用
        list.removeAll(StrIndex);

    }
    delete item ;

}

//将添加的联系人和群聊的分组关系写入到缓存文件
void CCMianWidget::WriteGroupingFile()
{

    auto i_r  = m_TreeGroupingItem.constBegin();
    for(; i_r != m_TreeGroupingItem.constEnd(); i_r++){

        QTreeWidgetItem* item = i_r.value();
        int itemSize = item->childCount(); //  子项的数量
        QStringList listName ;
        for(int i = 0; i < itemSize; i ++){
            QTreeWidgetItem* it = item->child(i);
            const QString Name = it->data(0, Qt::UserRole+1).toString();
            //qDebug()<< "当前要写入进缓存文件的数据为 - " << Name;
            listName.append(Name);
        }
        QString initName = i_r.key();
        m_TreeItem.insert(initName, listName);
    }

    //写入到JSON缓存文件
    QPeratioFile::instance()->SwitchContactGrouping(m_TreeItem,
                                                    QPeratioFile::instance()->getJsonContactFilURL());

    //将所有添加的联系人包含群聊添加到缓存文件中
     WriteGroupInformationFile(m_Contact);
}

//读取缓存文件中的联系人分组
void CCMianWidget::ReadingGrouping()
{

    QPeratioFile::instance()->readJsonContactGriuping(m_TreeItem,
                                                  QPeratioFile::instance()->getJsonContactFilURL());
    auto i_r = m_TreeItem.constBegin();
    for(; i_r != m_TreeItem.constEnd(); i_r++){
        const QString TreeName = i_r.key();
        if(!m_TreeGroupingItem.contains(TreeName)) addContactTree(TreeName);

        const QStringList list = i_r.value();
        if(list.isEmpty()){
            //qDebug()<<"数据测试";
        }
        for(const QString data : list){

            if(!m_Contact.contains(data)){
                qDebug()<< "拿取联系人分组缓存时，m_Contact未包含该成员";
            }
            QVariantMap params = m_Contact.value(data);
            QString sign = params["sign"].toString();
            QString strNamem =  params["GroupName"].toString() ;
            QString Account = params["GroupID"].toString();
            QString URL = params["URL"].toString();
            QImage image(URL);

            if(image.isNull()) {
                qDebug()<< "将联系人添加到分组，联系人头像路径加载失败" ;
                break ;
            }
             //qDebug()<<"当前已经进入 加载"<< TreeName;
            QTreeWidgetItem* item =  m_TreeGroupingItem.value(TreeName);
            addTreeItem(strNamem, sign, Account, image, item);
        }
    }

}

// 将图像数转换为2进制
QByteArray CCMianWidget::pixmapToBytes(const QPixmap & pix)
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    pix.save(&buf, "PNG"); // 可选 JPG、BMP
    buf.close();

    return ba;
}

//查询并返回头像路径
 QString CCMianWidget::JudgmentContactImage(const QString& uid)
{
    
    if (m_ContactImg.contains(uid)) {
        QPixmap pixmap; 
        pixmap.load(m_ContactImg.value(uid));
        if (!pixmap.isNull())
            return m_ContactImg.value(uid);
    }
    
    qDebug() << "头像路径中没有该成员头像 "; 
    return nullptr;
}

//拿取群信息数据(所有加入的群聊和添加的联系人列表)
void CCMianWidget::TakeoutGroupIdFile(const QString& URL, QMap<QString, QVariantMap>& memberMap)
{
    if (URL.isEmpty()) 
        return;
   QMap<QString, GroupMenber>member =  QPeratioFile::instance()->loadAllGroupMember(URL);
   QMap<QString, GroupMenber>::const_iterator i_r;
   i_r = member.constBegin();
   for (; i_r != member.constEnd(); i_r++) {
       QString Name = i_r.key();
       GroupMenber Menber = i_r.value();
       QVariantMap VarMap;
       VarMap["sign"] = Menber.Sign;
       VarMap["GroupName"] = Menber.QQNember;
       VarMap["GroupID"] = Menber.GroupId;
       VarMap["URL"] = Menber.URL;  

       memberMap.insert(Name, VarMap);
   }

}

//将群的基本信息写入JSON文件
bool CCMianWidget::WriteGroupInformationFile(const QMap<QString, QVariantMap>& memberMap)
{
    //将群信息写入进JSON文档
    QMap<QString, QVariantMap>::const_iterator i_r;
    i_r = memberMap.constBegin();
    QMap<QString, GroupMenber> Map;
    for (; i_r != memberMap.constEnd(); i_r++) {
        GroupMenber member;
        member.GroupId = i_r.key();     //群号
        member.MobileNember = " ";
        member.QQNember = i_r.value()["GroupName"].toString();
        member.Sign = i_r.value()["sign"].toString();   //个性签名
        member.URL = QPeratioFile::instance()->getImageURL(i_r.key());
        Map.insert(i_r.key(), member);
    }

    //写入 群信息数据到Json缓存文档
    bool isNull = QPeratioFile::instance()->saveGroupMemberToJson(Map, QPeratioFile::instance()->getJsonGroupFileURL());

    if (isNull) {
        qDebug() << "群信息数据缓存存储成功";
        return true; 
    }
    else {
        qDebug() << "群信息数据缓存存储失败";
        return false; 
    }
}

//接收来自服务器的联系人列表包含(加入的群聊)
void CCMianWidget::getServerContact(QString strData, const QVariantMap params)
{
    if (!strData.isEmpty() || !params.isEmpty()) {
        QMap<QString, QVariantMap> map; 
        map.insert(strData, params);

        //将群聊头像数据写入文件
        WriteGroupImageFile(strData, params["imgBin"].toByteArray());

        //将群的基本信息写入JSON文件
       WriteGroupInformationFile(map);
    }
    TakeoutGroupIdFile(QPeratioFile::instance()->getJsonGroupFileURL(), m_Contact);
}

 //接收来自服务器的用户信息数据
void CCMianWidget::getServerUserInformation(const bool &isEmpty, const QString &strName, const QByteArray &ImageData, const QString &strSign)
{

    if(isEmpty){
        QImage imaeg;
        if(imaeg.loadFromData(ImageData)){

            QPixmap pix;
            pix.load(":/Resources/MainWindow/head_mask.png");
            QPixmap pixmap =  getRoundImage(QPixmap::fromImage(imaeg), pix, FriendsDialog->getBel()->size());

            FriendsDialog->setUIWidget(strName, pixmap, strSign);
            FriendsDialog->show();

        }else{
            ui.searchLineEdit->clear();
            ui.searchLineEdit->setText("头像数据转换失败" );

        }

    }else{
        ui.searchLineEdit->clear();
       ui.searchLineEdit->setText("没有该用户请重新输入:");
    }

}

//接收来自服务器的添加好友的信息
void CCMianWidget::addServerFriendsInformation(const QString &strNamem, const QString &strSign,
                                               const QString &Account, const QByteArray &imageData)
{
    if(!m_Contact.contains(Account)){

    //没保存过该用户图像
    if(!m_ContactImg.contains(Account)){
         WriteGroupImageFile(Account, imageData);
        QString FileURL = QPeratioFile::instance()->getImageURL(Account);
         if(FileURL.isEmpty()){
            qDebug()<<"拿取添加好友的头像文件失败";
             return ;
         }
         m_ContactImg.insert(Account, FileURL);
    }

    QImage image(m_ContactImg.value(Account));
    if(image.isNull()){
        qDebug()<<"加载添加的好友头像数据失败";
        return ;
    }
    //添加到联系人树图根项
    addTreeItem(strNamem, strSign, Account, image, m_RootNodeContactTreeIteml);

    QVariantMap VarMap;
    VarMap["sign"] = strSign;
    VarMap["GroupName"] = strNamem;
    VarMap["GroupID"] = Account;
    VarMap["URL"] = m_ContactImg.value(Account);

    m_Contact.insert(Account, VarMap);
    }
    else{
        qDebug()<< "--- 已经添加过该好友 --- 用户为:  "<<  Account;
    }
}

//接收有用户请求添加自己为好友得请求
void CCMianWidget::addUserServer(const QString &strNamem, const QString &strSign, const QString &Account, const QByteArray &imageData)
{

    QImage imaeg;
    if(imaeg.loadFromData(imageData)){

        QPixmap pix;
        pix.load(":/Resources/MainWindow/head_mask.png");
        QPixmap pixmap =  getRoundImage(QPixmap::fromImage(imaeg), pix, FriendsAgree->getBel()->size());

        FriendsAgree->setUIWidget(strNamem, pixmap, Account);
        FriendsAgree->show();

}
}

 //右键菜单弹出
void CCMianWidget::ClickedTreGrouping(const QPoint &pos)
{
    QTreeWidgetItem* item = ui.treeWidget->itemAt(pos);
    if(! item) return ;
    //避免重复添加分组
    const QStringList list_item = m_Menu->getActionGroupingName();
    const QStringList list_map = m_TreeGroupingItem.keys();
    for(const QString& Name : list_map){
        if(!list_item.contains(Name)){
            m_Menu->addGroupingAction(Name);
        }
    }

    QString account = item->data(0, Qt::UserRole+1).toString();
    m_Menu->setCurrentAccount(account);
    m_Menu->setTreeItem(item);
    m_Menu->exec(ui.treeWidget->viewport()->mapToGlobal(pos));
}

 //将选中的联系人移动到其他分组
void CCMianWidget::MoveGroping(const QString &GroupingName)
{

    if(GroupingName.isEmpty()) return ;

    QTreeWidgetItem* item = m_TreeGroupingItem.value(GroupingName);
    if(!item) return;

    QString name = m_Menu->getTreeItem();

    if(m_Contact.contains(name)){

        DeleteTreeItem();   //删除当前项
        QVariantMap params = m_Contact.value(name);
        QString sign = params["sign"].toString();
        QString strNamem =  params["GroupName"].toString() ;
        QString Account = params["GroupID"].toString();
        QString URL = params["URL"].toString();
        QImage image(URL);
        if(image.isNull()) {
            qDebug()<< "将联系人添加到分组，联系人头像路径加载失败" ;
            return ;
        }

        addTreeItem(strNamem, sign, Account, image, item);
    }
}



//将群聊头像数据写入文件
void CCMianWidget::WriteGroupImageFile(const QString& GroupId, const QByteArray& ImagePkg)
{

    if(!GroupId.isEmpty() && !ImagePkg.isEmpty())
        QPeratioFile::instance()->WriteFileImage(GroupId, ImagePkg);
}

//将服务返还的图像数据写入进文件
void CCMianWidget::WriteImageFile(const QMap<QString, QByteArray>& map)
{

    QMap<QString, QByteArray>::const_iterator i_r;
    i_r = map.constBegin();
    for (; i_r != map.constEnd(); i_r++) {
        QPeratioFile::instance()->WriteFileImage(i_r.key(), i_r.value());
    }


}


//拿取群成员的缓存数据
void CCMianWidget::TakeoutGroupFile(const QMap<QString, QList<GroupMenber>>& memberList)
{
    const QList<GroupMenber>& List = memberList.value("2000");
    for (const GroupMenber& member : List) {
        QVariantMap params;
        
        params["GroupId"] = member.GroupId;
        params["MobileNember"] = member.MobileNember;
        params["QQNember"] = member.QQNember;
        params["Sign"] = member.Sign;
        params["status"] = '0';
        params["URL"] = member.URL;
        params["online"] = '1';

        m_Group.insert(member.MobileNember, params);
    }
}

//拿取群成员图像缓存数据
void CCMianWidget::TakeoutImageFile()
{
    
    QMap<QString, QVariantMap>::const_iterator i_r;
    i_r = m_Group.constBegin();
    for (; i_r != m_Group.constEnd(); i_r++) {
         QString Name = i_r.key();
         QString URL = i_r.value()["URL"].toString();
        
        QPixmap pixmap(URL);
      
        if (!pixmap.isNull()) {
            m_ContactImg.insert(Name, URL);
        }
        else
        {
            qDebug() << "当前加载" << Name << "的图像路径失败";
        }
        
    }


}

//将群成员对象写入进JSON文档
void CCMianWidget::WriteGroupFile(const QList<QStringList>&ListGroup)
{
    
    QList<GroupMenber> memberList;
    QString GrouId = "2000";    //所有的成员信息
    for (const QStringList list: ListGroup) {
        GroupMenber member;
        member.GroupId = list.at(0);
        member.MobileNember = list.at(1);
        member.QQNember = list.at(2);
        member.Sign = list.at(3);
        member.URL = QPeratioFile::instance()->getImageURL(member.MobileNember);

        memberList.append(member);
    }

    //写入JSON文档当中
    bool isNull =  QPeratioFile::instance()->saveGroupMemberToJson(GrouId, memberList);
    if (isNull)
        qDebug() << "群成员JSON文档写入成功";
    else
        qDebug() << "群成员JSON文档写入失败";


}



//拿取缓存数据
void CCMianWidget::TakeoutFile()
{
    
    //拿取群成员缓存数据
    TakeoutGroupFile(QPeratioFile::instance()->loadAllGroupMember());

    //拿取群信息数据(所有加入的群聊和添加的联系人列表)
    TakeoutGroupIdFile(QPeratioFile::instance()->getJsonGroupFileURL(), m_Contact);
    
    //拿取图像缓存数据
    TakeoutImageFile();
}

//判断成员缓存数据是否已经存在
bool CCMianWidget::JudgmentFile()
{
    const QString& JsonUrl = QPeratioFile::instance()->getJsonFileURL();
    const QString& ImageUrl = QPeratioFile::instance()->getJsonGroupFileURL();

    QFile JsonFile(JsonUrl);
    QFile ImageFile(ImageUrl);

    //return false;

    if (JsonFile.exists() && ImageFile.exists())
        return true;
    else
        return false;
}

//加载缓存数据
void CCMianWidget::LoadingFile()
{
    QString Account = gLoginEmployeeID;

    QImage image(m_ContactImg.value(Account));
    if (!image.isNull()) {
        QPixmap pix;
        pix.load(":/Resources/MainWindow/head_mask.png");
        ui.headLabel->setPixmap(getRoundImage(QPixmap::fromImage(image), pix, ui.headLabel->size()));
    }

    QMap<QString, QVariantMap>::const_iterator i_r;
    i_r = m_Contact.constBegin();
    for (; i_r != m_Contact.constEnd(); i_r++) {
        QVariantMap params = i_r.value();
        QString GroupId = params["GroupID"].toString(); 
        QString Sign = params["sign"].toString();
        QString GroupName = params["GroupName"].toString();
        QString URL = params["URL"].toString();
        QImage Imaeg(URL);
        if (!Imaeg.isNull()) {

           // addTreeItem(GroupName, Sign, GroupId, Imaeg, m_RootNodeGroupTreeItem);
        } 
    }

    
}

//将联系人分组项写入到JSON文件
void CCMianWidget::WriteContactGrouping(const QMap<QString, QList<QString>> &map)
{
    bool isEmpty =  QPeratioFile::instance()->SwitchContactGrouping(map,
                 QPeratioFile::instance()->getJsonContactFilURL());
    if(!isEmpty)
        qDebug() <<"联系人分组缓存写入到JSON文化失败";

}





//拿取服务器返回的账号数据
void CCMianWidget::getServerAccount(QMap<QString, QString> map, QString strGroupId, QString strSign)
{  
    m_Account = map; 
    m_GrouId = strGroupId;
    m_Sign = strSign;
    QString strAccount;
    
    if (m_Account.contains(m_accout)) {
        strAccount = m_accout;
        //gLoginEmployeeID = m_Account.value(m_accout);
    }
    else
    {
        //gLoginEmployeeID = m_accout;
        strAccount = m_Account.key(m_accout);
    }
    
    if (JudgmentFile()) {   //判断是否保存过数据缓存
        TakeoutFile();      //拿取缓存数据
        LoadingFile();      //加载缓存数据
        // 发送当前所有联系人缓存数据拿取完成，通知加载联系人分组项目
        emit NotificationsGroupingLoafingUI();

    }
    else {                  //没有数据缓存直接向服务器请求数据
        //拿取用户头像
        TcpClient::getInstance()->getServerUserImage(strAccount);

        TcpClient::getInstance()->getServerUserGroupID(strAccount, strGroupId);
        TcpClient::getInstance()->getServerUserGroupID(strAccount, "2000");
        //拿取群聊成员数据
        //TcpClient::getInstance()->getServerUserGroup("2000");
        //拿取群聊成员数据
        TcpClient::getInstance()->getServerUserGroup(strAccount);
    }


}


//尺寸事件
void CCMianWidget::resizeEvent(QResizeEvent* event)
{
    setUserName(QString::fromUtf8("白纸—王老头"));
    BasicWindow::resizeEvent(event);

}
//键盘事件
bool CCMianWidget::eventFilter(QObject* obj, QEvent* event)
{

    if(obj == ui.searchBtn && event->type() == QEvent::MouseButtonPress){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton){
            QString strName = ui.searchLineEdit->text();
            m_Text = ui.searchLineEdit->text(); //拿取记录
            if(!strName.isEmpty()){
                 //FriendsDialog->show();
                TcpClient::getInstance()->getServerUserInformation(gLoginEmployeeID, strName);

            }
        }

    }

    // 点击搜索框
    if (ui.searchLineEdit == obj) {
        //键盘焦点事件
        if (event->type() == QEvent::FocusIn) {
            //这里加一个Widget
            QString colorR = QString::number(m_colorBackGround.red());
            QString colorG = QString::number(m_colorBackGround.green());
            QString colorB = QString::number(m_colorBackGround.blue());

            ui.searchLineEdit->setStyleSheet(
                QString("QLineEdit#searchLineEdit {"
                        "  background-color: rgb(255,255,255);"
                        "  border-bottom: 1px solid rgba(%1, %2, %3, 0.4);"
                        "}")
                    .arg(colorR).arg(colorG).arg(colorB)
                );
            ui.searchBtn->setStyleSheet(R"(
    #searchBtn {
        background-image: url(:/Resources/MainWindow/bottom/find.png);
        background-repeat: no-repeat;
        background-position: center;
        border: none;
        padding: 0px;
    }
    #searchBtn:pressed {
        background-image: url(:/Res ources/MainWindow/bottom/find_down.png);
    }
    #searchBtn:hover {
        background-image: url(:/Resources/MainWindow/bottom/find_hover.png);
    }
)");
        }
        else if (event->type() == QEvent::FocusOut) {
            //清空样式
            updateSeachStyle();
        }
    }

    return false;
}

void CCMianWidget::mousePressEvent(QMouseEvent* event)
{
    //当没有选择LineEdit时同时他又具有焦点
    
    if (qApp->widgetAt(event->pos()) != ui.searchLineEdit && ui.searchLineEdit->hasFocus())
    {
        //清除焦点
        ui.searchLineEdit->clearFocus();
        }
    else if (qApp->widgetAt(event->pos()) != ui.lineEdit && ui.lineEdit->hasFocus()) {
        ui.lineEdit->clearFocus();
    }
    BasicWindow::mousePressEvent(event);
}

//获取登录者的头像
QString CCMianWidget::getHeadPicturePath()
{
    return nullptr;
}

//点击
void CCMianWidget::onItemClicked(QTreeWidgetItem* itme, int column)
{
    bool bIsChild = itme->data(0, Qt::UserRole).toBool();
    if (!bIsChild) {
        itme->setExpanded(!itme->isExpanded());
    }

}

//展开
void CCMianWidget::onItemExpanded(QTreeWidgetItem* item)
{
    bool bIsChild = item->data(0, Qt::UserRole).toBool();
    if (!bIsChild) {
        //dynamic_cast 将基类对象指针或引用转换到继承类指针
        RootContatItem* prootItem = dynamic_cast<RootContatItem*>(ui.treeWidget->itemWidget(item,0));
        if (prootItem) {
            prootItem->setExpanded(true);
        }
    }
}

//收缩
void CCMianWidget::onItemCollapsed(QTreeWidgetItem* item)
{
    bool bIsChild = item->data(0, Qt::UserRole).toBool();
    if (!bIsChild) {
        //dynamic_cast 将基类对象指针或引用转换到继承类指针
        RootContatItem* prootItem = dynamic_cast<RootContatItem*>(ui.treeWidget->itemWidget(item, 0));
        if (prootItem) {
            prootItem->setExpanded(false);
        }
    }
}

//双击
void CCMianWidget::onItmeDoubleClicked(QTreeWidgetItem* item, int column)
{

    bool bIsChild = item->data(0, Qt::UserRole).toBool();
    if (bIsChild) {
       QString name=  item->data(0, Qt::UserRole + 1).toString();
       //通过QQ号或者群号拿取当前的群聊或者用户信息
       const QVariantMap& params = m_Contact.value(name);
       
       const QString& Sign = params["sign"].toString();           //拿取 个性签名
       const QString& Name = params["GroupName"].toString();      //拿取 名称
       const QString& image = params["URL"].toString(); //拿取 头像
      // const QStringList& ListGroup = m_Group.value(name);       //拿取所有的群聊成员信息数据  

       WindowManager::getInstance()->
           addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString(),
               Name,
               Sign,
               image,
               m_Group);
               
    }

}

//接收服务端返回的群聊成员数据
void CCMianWidget::onGetServerGoup(QList<QStringList>list, int rows, int colunms, const QMap<QString, QByteArray>& map)
{

    if (m_Group.isEmpty() && m_ContactImg.isEmpty()) { //这 2项数据都未初始化过时

        QMap<QString, QByteArray> pkg = map;    //群成员手机号与头像数据的映射
       
        //拿取客户端所在的群聊号 
        m_groupId.append(list[0][0]);
     
        //将图像数据写入文件
        WriteImageFile(pkg);
        //将群聊信息写入进JSON文件
        WriteGroupFile(list);
    }
    //拿取群成员缓存数据
    TakeoutGroupFile(QPeratioFile::instance()->loadAllGroupMember());

}
//app 的公能实现接口
void CCMianWidget::onAppIconClicked()
{
    //判断信号发送者得对象名是否是app_skin
    if (sender()->objectName() == "app_skin") {
        SkinWindow* skinwindow = new SkinWindow;
        skinwindow->show(); 
    }
}
