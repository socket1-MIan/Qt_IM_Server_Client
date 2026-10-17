#include "TcpClient.h"
#include "QPeratioFile.h"

#include <QDebug>
#include <QHostAddress>
# include <QBuffer>
# include <QMessageBox>
# include <QVariantMap>


//创建全局静态对象， 单列模式
//Q_GLOBAL_STATIC(TcpClient, Client)


// 消息头结构体：自定义TCP数据包头部，固定长度
struct MessageHeader
{
    uint32_t magic;        // 魔数：校验数据包合法性，区分不同协议
    uint16_t version;      // 协议版本号，用于协议迭代兼容
    uint16_t cmd;          // 指令码，区分不同业务请求/响应
    uint32_t bodyLength;   // 数据体长度，用于拆包、判断包完整性
    uint32_t seq;          // 消息序列号，用于请求应答配对、防重复、追踪报文
};


// 魔数定义
constexpr quint32 MAGIC_NUM = 0x5A5A5A5A;
// 协议版本
constexpr quint16 PROTO_VER = 1;
// 数据流版本统一固定（Qt6.5）
constexpr QDataStream::Version DATA_STREAM_VER = QDataStream::Qt_6_5;

// 全局消息序列号，每发送一个数据包自增1
uint32_t m_seqCounter = 0;

extern QString gLoginEmployeeID; //登录者的手机号码

/**
 * @brief 组装完整请求数据包
 * @param cmd 业务指令码
 * @param params 业务参数键值对
 * @return 完整二进制数据包(头部+数据体)
 */

QByteArray buildRequest(uint16_t cmd, const QVariantMap& params) {

    // 1. 序列化业务参数为包体
    QByteArray body;
    QDataStream out(&body, QIODevice::WriteOnly);
    out.setVersion(DATA_STREAM_VER);
    out << params;

    // 2. 填充消息头
    MessageHeader header;
    header.magic = MAGIC_NUM;
    header.version = PROTO_VER;
    header.cmd = cmd;
    header.bodyLength = body.size();
    header.seq = m_seqCounter++;

    // 3. 拼接包头+包体
    QByteArray pkg;
    pkg.append(reinterpret_cast<const char*>(&header), sizeof(MessageHeader));
    pkg.append(body);

    return pkg;
}

/**
 * @brief 单包解析函数（外部传入已缓存拼接好的缓冲区，内部拆出单包）
 * @param data 原始缓冲区引用，解析完成后剔除已处理数据
 * @param outParams 解析成功输出参数
 * @param outCmd 输出指令码
 * @return true=解析出一个完整包，false=半包/脏数据
 */

bool parsePackage(QByteArray& data, uint16_t& outCmd, QVariantMap& outParams) {

    // 计算消息头结构体占用字节大小
    const int headerSize = sizeof(MessageHeader);
    // 循环：只要当前缓冲区总字节数 >= 单个包头长度，就有机会解析出完整数据包
    while (data.size() >= headerSize) {
        // 取缓冲区首地址，强制类型转换读取头部数据（拿到最前面的包头）
        MessageHeader header = *(reinterpret_cast<const MessageHeader*>(data.constData()));

        // 校验魔数，判断是不是我们协议的数据包
        if (header.magic != MAGIC_NUM) {
            // 魔数不匹配，说明起始字节是乱码/脏数据，删除第一个字节，下次循环重新校验
            data.remove(0, 1);
            continue;

        }
        // 整个数据包总长度 = 头部固定长度 + 业务体长度
        quint32 totalPkgLen = headerSize + header.bodyLength;

        // 判断：缓冲区剩余数据不足以容纳一整个完整数据包 → 半包，直接退出等待后续数据
        if ((quint32)data.size() < totalPkgLen)
            return false;

        // 从缓冲区头部截取【一整个完整数据包】
        QByteArray onePkg = data.left(totalPkgLen);
        // 把已经解析完成的数据从缓冲区删掉，避免重复解析
        data.remove(0, totalPkgLen);
        // 跳过头部字节，截取真正的业务数据体
        QByteArray body = onePkg.mid(headerSize, header.bodyLength);
        // 构造数据流，设置统一版本反序列化
        QDataStream in(body);
        in.setVersion(DATA_STREAM_VER);
        // 把二进制包体还原成 QVariantMap 键值对
        in >> outParams;
        // 输出当前数据包指令码
        outCmd = header.cmd;

        // 成功解析一个完整包，返回true
        return true;
    }

    // 走到这里：缓冲区长度不足一个包头，属于半包，暂时无法解析
    return false;
}

// 构造函数：绑定套接字信号与对应槽函数
TcpClient::TcpClient(QObject* parent):
	QObject(parent)
{
	// 当m_socket成功连接服务器，自动执行onConnected
	connect(&m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
	// 当套接字缓存区接收到新数据，自动执行onReadyRead收包解析
	connect(&m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
	// 连接断开时，执行onDisconnected做清理
	connect(&m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
	// 套接字产生网络错误时，执行错误处理函数
	connect(&m_socket, &QTcpSocket::errorOccurred, this, &TcpClient::onErrorOccurred);
    


    //initVideWidget(); //初始 化 视频套件
}

//发起连接服务端
void TcpClient::connectToServer(const QString& ip, quint16 port) {

	// 判断当前是否已经处于连接状态，避免重复发起连接
	if (m_socket.state() == QTcpSocket::ConnectedState) {

		qDebug() << " 已连接无需重复连接";
		return;
	}

	//发起异步连接请求
	m_socket.connectToHost(QHostAddress(ip), port);

}

// 对外发送消息接口
void TcpClient::sendMsg(uint16_t cmd, const QVariantMap& params) {

	// 校验连接状态，未连接不能发送
	if (m_socket.state() != QTcpSocket::ConnectedState) {
		qDebug() << "未连接服务端，发送失败";
		return;
	}

	// 调用协议打包函数生成完整数据包
	QByteArray pkg = buildRequest(cmd, params);
    qDebug() << "客户端发送指令" << cmd;
	//写入套接字发送数据
	m_socket.write(pkg);

}

//初始化信号连接
void TcpClient::initSignalBind()
{
    /* 防止2个单列相互调用导致递归 */

    //当管理员窗口修改连接属性配置的时候， 进行重连
    connect(AdministratorWidget::getInstance(), &AdministratorWidget::serverConfigChanged,
            this, &TcpClient::connectToServer);
}

// 主动断开连接
void TcpClient::disconnectServer() {

	//发起断开
	m_socket.disconnectFromHost();
	//清空接收缓冲区残留数据
	m_recvBuf.clear(); 
}

//当前是否连接上服务器
bool TcpClient::ServerSuccess()
{
    if (m_socket.state() == QTcpSocket::ConnectedState)
        return true;
    else
        return false;
}

//向服务端验证登录
void TcpClient::veryfyAccountServerCode(const QString strAccount, const QString strCode)
{
    //先保存当前输入的账号数据，待进行验证
    m_strAccountInput = strAccount;

    uint16_t cmd = UERY_SQL_ACCOUNT_USER; 
    QVariantMap params;

    params["Account"] = strAccount;
    params["code"] = strCode; 

    sendMsg(cmd, params);

}

//向服务端发送返回客户端手机和 QQ号的请求
void TcpClient::getServerUserID(const QString strId)
{

    uint16_t cmd = USERY_SQL_ACCOUNTID;
    QVariantMap params; 
    params["Account"] = strId;

    sendMsg(cmd, params);
}

//向服务端发送登录成功的回执
void TcpClient::sendServerLoginSuccess(const QString &strAccount)
{
    uint16_t cmd = LOGN_SERVER_SUCESS;
    QVariantMap params;
    params["Account"] = strAccount;

    sendMsg(cmd, params);

}

//向服务端发送查询验证并返回的图像数据
void TcpClient::getServerUserImage( QString strAccountInput)
{
   // bool isok = m_strAccountID.contains(strAccountInput);
    //if (!isok)
       // strAccountInput = m_strAccountID.value(strAccountInput);

   
    uint16_t cmd = USERY_SQL_IAMGE_USER;
    QVariantMap params;
    params["Account"] = strAccountInput; 

    sendMsg(cmd, params);

}

//向服务端发送请求群成员用户数据
void TcpClient::getServerUserGroup(const QString strAccountInput)
{

    uint16_t cmd = USERY_SQL_GROUP;
    QVariantMap params; 
    params["Account"] = strAccountInput;
    sendMsg(cmd, params);
}

//向服务端发送请求群聊信息数据
void TcpClient::getServerUserGroupID(const QString& strGroupInput, const QString& strAccountInput)
{

    if (strGroupInput.isEmpty() || strAccountInput.isEmpty()) {
        qDebug() << "向服务端发送请求群聊信息数据失败";
        return; 
    }

    uint16_t cmd  = USERY_SQL_GROUP_USER;
    QVariantMap params; 
    params["Account"] = strGroupInput;
    params["GroupID"] = strAccountInput;

    sendMsg(cmd, params);

}

//接收来自服务端的群聊信息数据
void TcpClient::addServerUserGroup(const QVariantMap& params)
{
    QString sign = params["sign"].toString();
    QByteArray imagData = params["imgBin"].toByteArray();
    int imageSize = params["imgSize"].toInt(); 
    QString GropName = params["GroupName"].toString(); 
    QString GroupID = params["GroupID"].toString();

    QImage image; 
    if (image.loadFromData(imagData)) {
        emit QueryServerGroupID(GropName, sign, GroupID, image);
        //返回从服务端接收到的已经加入和添加的联系人或者群聊列表
        emit QueryServerContact(GroupID, params);
    }
    else {
        qDebug() << "接收群聊成员数据失败";
        
    }

}

//向指定的群聊发送消息数据
void TcpClient::SendGroupId(const ChatMsg& msg)
{
    //ChatMsg 结构体数据转换为QByteArrey
    QVariantMap params = QPeratioFile::instance()->ChatMsgToQVariantMap(msg);

    uint16_t cmd = SERVER_GROUP; 

    sendMsg(cmd, params);
}

//向指定联系人发送消息数据
void TcpClient::SendUser(const ChatMsg &msg)
{

    //ChatMsg 结构体数据转换为QByteArrey
    QVariantMap params = QPeratioFile::instance()->ChatMsgToQVariantMap(msg);

    uint16_t cmd = SERVER_USER;

    sendMsg(cmd, params);
}

//向服务器发送添加好友的申请
void TcpClient::addServerUserFriends (const QString &addAccount, const QString &Account)
{

    QVariantMap params;
    params["addAccount"] = addAccount ;
    params["Account"] =  Account;
    uint16_t cmd =SEVER_ADDUSER_FRIENDS;

    sendMsg(cmd, params);

}

//向服务器请求用户的个人信息
void TcpClient::getServerUserInformation(const QString& Account, const QString& addAccount)
{
    QVariantMap params;
    params["Account"] = Account;
    params["addAccount"] = addAccount;
    uint16_t cmd = USERY_SQL_INFORMATION;

    sendMsg(cmd, params);

}

//向服务器发送同意添加好友得申请
void TcpClient::addArreeAddUser(const QString &Account, const QString &addAccount)
{
    QVariantMap params;
    params["Account"] = Account;
    params["addAccount"] = addAccount;
    uint16_t cmd = SERVER_USER_AGREE;

    sendMsg(cmd, params);
}

//将图像数转换为2进制
QByteArray TcpClient::pixmapToBytes(const QPixmap& pix)
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    pix.save(&buf, "PNG"); // 可选 JPG、BMP
    buf.close();

    return ba;
}


 //追加到日志界面
void TcpClient::addLogGing(const QString &strData)
{
    auto pAdmin = AdministratorWidget::getInstance();
    if (!pAdmin)
    {
        qDebug() << "管理员窗口单例为空";
        return;
    }

    QMetaObject::invokeMethod(pAdmin,
                              "appendLog",
                              Qt::QueuedConnection,
                              Q_ARG(QString, strData));
}

//新键用户
void TcpClient::NewUser(const QString &Accoutn, const QString &strName)
{

    uint16_t cmd = NEW_SQL_USER;
    QVariantMap params;
    params["Account"] = Accoutn;
    params["Name"] = strName;

    sendMsg(cmd, params);
}

//向服务端请求离线消息数据
void TcpClient::getOfflineMsg(const QString &Account)
{
    uint16_t cmd = OFFLINE_NSG_PUSH;
    QVariantMap params;
    params["Account"] = Account;

    sendMsg(cmd, params);

}

//用户拉取到离线消息向服务器发送回执
void TcpClient::sendServerOfflineMagMuccess(const QString &Account)
{

    uint16_t cmd = GET_OFFLINE_MSG_SUCC;
    QVariantMap params;
    params["Account"] = Account;

    sendMsg(cmd, params);

}

 //用户向客户端发送消息已读回执
void TcpClient::sendServerReanMsg(const QString &sendAccount, const QString &recerAccount)
{
    if(sendAccount.isEmpty() || recerAccount.isEmpty()){
        return ;
    }

    uint16_t cmd =  MSG_READ_RECEIPT ;
    QVariantMap params;
    params["sendAccount"] = sendAccount;
    params["recerAccount"] = recerAccount;

    sendMsg(cmd, params);

}

 //向服务端2次拉取图像离线消息2进制数据
void TcpClient::getServerOfflineMsgImaeg(const QString &Account, const QString &ImageName)
{
    uint16_t cmd = GET_OFFLINE_MSG_IMAGE;
    QVariantMap params;
    params["Account"] = Account;
    params["ImageName"] = ImageName;

    sendMsg(cmd, params);

}

//初始化视频通话部件
void TcpClient::initVideWidget()
{

    m_localAccount = "user1";
    VideoWidget = new  QDialog();
    VideoWidget->setFixedSize(640, 480);
    qDebug()<<"WA_DeleteOnClose = "<< VideoWidget->testAttribute(Qt::WA_DeleteOnClose);
    m_videoUdp = new VideoCallUdp(VideoWidget);
    m_remoteView = new VideoRenderWidget(VideoWidget);

    QVBoxLayout* layout = new QVBoxLayout(VideoWidget);
    layout->addWidget(m_remoteView);
    // 关键：让视频控件抢占所有剩余空间
    layout->setStretchFactor(m_remoteView, 1);

    m_videoUdp->initPort();   //只初始化端口和ip绑定
    // 信号绑定【最终标准写法】
    connect(m_videoUdp, &VideoCallUdp::sigRemoteVideoFrame,
            m_remoteView, &VideoRenderWidget::onRecvFrame,
            Qt::QueuedConnection);
    // 监听窗口销毁，把全部指针置nullptr！重要！
    connect(VideoWidget, &QDialog::destroyed, this, [=](){
        qDebug() << "VideoWidget窗口被销毁，清空所有指针";
        VideoWidget = nullptr;
        m_videoUdp = nullptr;
        m_remoteView = nullptr;
        m_localView = nullptr;
    });
    VideoWidget->hide();
}

void TcpClient::initVideWidget(QWidget* widget, QVBoxLayout* VBoxLayout)
{
    if(!widget || !VBoxLayout) {
        qDebug() <<"初始化当前视频窗口失败" ;
        return ;
    }

    m_localAccount = "user1";

    m_videoUdp = new VideoCallUdp(widget);
    m_remoteView = new VideoRenderWidget(widget);

    VBoxLayout->addWidget(m_remoteView);
    // 关键：让视频控件抢占所有剩余空间
    VBoxLayout->setStretchFactor(m_remoteView, 1);

    m_videoUdp->initPort();   //只初始化端口和izp绑定
    // 信号绑定【最终标准写法】
    connect(m_videoUdp, &VideoCallUdp::sigRemoteVideoFrame,
            m_remoteView, &VideoRenderWidget::onRecvFrame,
            Qt::QueuedConnection);
    // 监听窗口销毁，把全部指针置nullptr！重要！
    connect(widget, &QDialog::destroyed, this, [=](){
        qDebug() << "VideoWidget窗口被销毁，清空所有指针";
        VideoWidget = nullptr;
        m_videoUdp = nullptr;
        m_remoteView = nullptr;
        m_localView = nullptr;
    });
    //VideoWidget->hide();
    emit sinVideHide();

}

 //主动发起通话呼叫
void TcpClient::slotCall()
{
    QString target = "10005";
    // TCP发送呼叫邀请
    QVariantMap msg = createVideoSignal(Cmd_VideoCall_Invite, gLoginEmployeeID, target, "");
    uint16_t cmd = Cmd_VideoCall_Invite;
    qDebug() << "正在发送通话呼叫" << cmd;
    m_callState = Call_WaitAccept; // 我是通话状态发起人
    sendMsg(cmd, msg);

}

//接收到通话请求
void TcpClient::onRecvSignal(const QVariantMap &map)
{
  //解析数据字段
    uint16_t cmd = map["cmd"].toInt();// 指令码
    QString fromUser = map["fromAccount"].toString();//消息发送者的账号
    QString toUser = map["toAccount"].toString();   //消息接收者账号
    QString payload = map["payload"].toString();    //对方ip加端口号

    //校验是否是本机的信令信息
    if(toUser != gLoginEmployeeID)
        return ;    //不是本机的信息直接returan

    switch(cmd){
        case Cmd_VideoCall_Invite: //向本机发送视频邀请
        {

            receiveCall(fromUser);

            qDebug() <<"已经接收到通话请求" ;
            break;
        }

        case    Cmd_VideoCall_Accept:
            // 仅我处于等待接听状态才生效
            if (m_callState == Call_WaitAccept)
            {
                // 单纯应答标记，暂不做推流，等待对方UDP地址
            }
            break;
        case    Cmd_VideoCall_UdpAddr://    接收对方的UDP账号推送
        {

            receiveIpPort(fromUser, payload);

            break;
        }
        case Cmd_VideoCall_Close://       挂断通话
        {
           sotpCall();
            break;
        }
        case Cmd_VideoCall_Reject://       拒绝通话
        {

            break;
        }
    }
}

// 发起通话请求
void TcpClient::slotCall(QString Account)
{
    //QString target = "10005";
    // TCP发送呼叫邀请
    if(m_callState == Call_Idle){   //空闲状态

    QVariantMap msg = createVideoSignal(Cmd_VideoCall_Invite, gLoginEmployeeID, Account, "");
    uint16_t cmd = Cmd_VideoCall_Invite;
    qDebug() << "正在发送通话呼叫" << cmd;
    m_callState = Call_WaitAccept; // 我是通话状态发起人
    sendMsg(cmd, msg);
    }
}

//挂断通话
void TcpClient::sotpCall()
{
    // 当前是否为通话状态
    if(m_callState == CallState::Call_Connected){
        QVariantMap msg = createVideoSignal(Cmd_VideoCall_Close, gLoginEmployeeID, m_peerAccount, "");
        uint16_t cmd = Cmd_VideoCall_Close;
        qDebug() << "发送挂断通话指令" << cmd;
        m_callState = Call_Idle; //空闲通话状态
        sendMsg(cmd, msg);
        m_videoUdp->stopCall();
    }
}

//接收到通话状态请求
void TcpClient::receiveCall(const QString fromUser)
{

    //判断本机是否是正在通话
    if(m_callState != Call_Idle){// 我发起呼叫，等待对方接听
        QVariantMap busy = createVideoSignal(Cmd_VideoCall_Reject,
                                             gLoginEmployeeID, fromUser, "busy");
        sendMsg(Cmd_VideoCall_Reject ,busy);
        qDebug()<< "当前正在通话";
        return ;
    }

    m_callState = Call_InComing;// 收到别人来电，等待我接听
    m_peerAccount = fromUser;   // 对方的手机号

    QMessageBox::StandardButton ret = QMessageBox::question(
        nullptr,
        "视频呼叫",
        QString("用户 %1 请求视频通话，是否接听？").arg(fromUser),
        QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No)
        );

    if(ret == QMessageBox::Yes){
        // 点击接听：1、回复Accept同意信令；2、把本机UDP地址发给对方
        QVariantMap acceptMsg =  createVideoSignal(Cmd_VideoCall_Accept,
                                                  gLoginEmployeeID, fromUser, "");

        uint16_t Ucmd =Cmd_VideoCall_Accept ;
        sendMsg(Ucmd, acceptMsg);   //发起呼叫指令

        //发送本机的 ip地址端口号

        QString localIppot = m_videoUdp->getLocalPort();      // 获取本机局域网 IP
        QString localIp = m_videoUdp->getLocalIp();
        QString localUdpAddr = QString("%1:%2").arg(localIp).arg(localIppot) ;

        QVariantMap udpAddrMsg = createVideoSignal(Cmd_VideoCall_UdpAddr,
                                                   gLoginEmployeeID, fromUser, localUdpAddr);
        sendMsg(Cmd_VideoCall_UdpAddr,udpAddrMsg);

        emit sinVideShow();
        //VideoWidget->show();
    }
}

//接收到对方ip端口推送
void TcpClient::receiveIpPort(const QString fromUser, const QString payload)
{
    //拿取ip地址和端口号
    QStringList addList = payload.split(":", Qt::SkipEmptyParts);
    //必须拆分出ip和端口号
    qDebug() << "原始payload:" << payload;
    if(addList.size() == 2){
        QString remoteIp = addList[0];
        quint16 remotePort = addList[1].toUShort();
        m_socketIP = remoteIp ;
        m_prot = remotePort;

        qDebug() <<"IP地址为 = " << m_socketIP;
        qDebug() << "端口号为 = " << m_prot;
        qDebug() << "m_videoUdp 指针地址 = " << m_videoUdp;

        m_videoUdp->setRemoteAddr(remoteIp, remotePort);

        // ========== 关键区分角色 ==========
        // 角色1：我是【主叫】：拿到被叫地址后，必须回传我的地址给对方
        if (m_callState == Call_WaitAccept){

            //发送本机的 ip地址端口号
            QString localIppot = m_videoUdp->getLocalPort();      // 获取本机局域网 IP
            QString localIp = m_videoUdp->getLocalIp();
            QString localUdpAddr = QString("%1:%2").arg(localIp).arg(localIppot) ;

            QVariantMap udpAddrMsg = createVideoSignal(Cmd_VideoCall_UdpAddr,
                                                       gLoginEmployeeID, fromUser, localUdpAddr);
            sendMsg(Cmd_VideoCall_UdpAddr,udpAddrMsg);
            // 切换为通话连接状态
            m_callState = Call_Connected;
            qDebug() << "准备执行initCapture";
            m_videoUdp->initCapture(640, 480);
            qDebug() << "initCapture 执行完成";
            m_videoUdp->startCapture(); // 开启向外推流
            qDebug() << "startCapture 执行完成";

        }
        // 角色2：我是【被叫】：收到主叫回传的地址，直接进入通话状态即可
        else {
            m_callState = Call_Connected;
            qDebug() << "准备执行initCapture";
            m_videoUdp->initCapture(640, 480);
            qDebug() << "initCapture 执行完成";
            m_videoUdp->startCapture(); // 开启向外推流
            qDebug() << "startCapture 执行完成";
        }
        emit sinVideShow();
        //VideoWidget->show();
    }

    return ;

}

// 返回单列对象
TcpClient* TcpClient::getInstance()
{

    static TcpClient Client;
    return &Client;
}

//连接成功的回调
void TcpClient::onConnected() {
	qDebug() << "客户端连接服务端成功";
    m_heartbeatTimer.setInterval(HEARTBEAT_INTERVAL_MS);
    connect(&m_heartbeatTimer, &QTimer::timeout,
            this, &TcpClient::slotSendHeartbeat);
    m_heartbeatTimer.start();


    //发送连接成功的信号
    emit SuccessServer(true);
}

// 收到数据入口，处理粘包、拆包、解析业务
void TcpClient::onReadyRead() {

	// 读取当前套接字所有收到数据，追加到缓冲区
	m_recvBuf.append(m_socket.readAll());
   
	uint16_t cmd;
	QVariantMap params;
  
	// 循环解析缓冲区，只要解析出完整包就持续处理
	while (parsePackage(m_recvBuf, cmd, params)) {

        switch (cmd) {
            case 0x0100: {
                //取出群号参数
                int groupId = params["groupID"].toInt();
                QString Loggin = "客户端收到服务端应答, 群号:" + QString::number(groupId) + "指令: " + cmd;
                qDebug() <<Loggin ;

                addLogGing (Loggin);
                break; 
            }
                //查询验证失败
            case ERROR_SQL_ACCOUNT: {
              
                QString Error = params["Error"].toString();
                qDebug() << Error << " : " << cmd;
                //发送验证失败后的信号
                emit QuerySercerCode(false);
                break;
            }
                //查询登录验证成功
            case RETURN_SQL_ACCOUNT: {
              
               //验证是否为自己的包
                if (m_strAccountInput == params["Account"].toString()) {
                    
                    QString Correct = params["Correct"].toString();
                    //拿取当前登录的账号
                    m_strAccountInput = params["Account"].toString();

                    qDebug() << Correct << " : " << cmd;

                    QString Loggin = "服务器验证账号密码成功, 指令: " + cmd;
                    addLogGing(Loggin);
                    //发送验证成功后的信号
                    emit QuerySercerCode(true);
                }
                break;
            }
            case USERY_SQL_ACCOUNTID://接收服务端返回的手机号和 QQ号数据
            {
                //验证是否是发送给当前客户端的数据包
                if (m_strAccountInput == params["Account"].toString()
                    || m_strAccountInput == params["Moblie"].toString()) {

                    QString strAccont = params["Account"].toString(); //QQ号
                    QString strMoblie = params["Moblie"].toString(); //手机号
                    QString strGroupId = params["GroupID"].toString();//群号
                    QString strSign = params["Sign"].toString();      //个性签名

                    this->m_strAccountId = strAccont;
                    this->m_strMoblie = strMoblie;

                    //向服务器发送请求离线消息的请求
                    getOfflineMsg(m_strMoblie);

                    m_strAccountID.insert(strAccont, strMoblie);

                    const QString str = "接收服务端返回的手机号和 QQ号数据, 指令: " +  cmd;
                    addLogGing(str);

                    emit QUeryServerAccount(m_strAccountID, strGroupId, strSign);
                }

                break;  
            }
            case USERY_SQL_IAMGE_USER: { //接收务器返回给客户端的头像图像数据

                //验证是否是发送给当前客户端的数据包
                if (params["Account"].toString() == m_strAccountId
                    || params["Account"].toString() ==m_strMoblie) {
                    
                    qDebug() << "接收到的图像数据";
                    addLogGing(QString("接收到的图像数据 cmd：" + QString::number(cmd)));
                    const QString strAccountInput = params["Account"].toString();
                    QByteArray imgBin = params["imgBin"].toByteArray();
                    m_ImageUser = imgBin;
                    const QString iamgeName = params["imgName"].toString();
                    QImage recvImage;
                    if (recvImage.loadFromData(imgBin)) {
                        //返回从服务器接收到的图像数据
                        emit QueryServerImage(recvImage);
                    }
                    else {
                        emit QueryServerImage(QImage());
                    }
                }
                break; 
            }
            case USERY_SQL_GROUP://接收务器返回给客户端群聊用户数据
            {

                QString str = params["Account"].toString(); 
                //验证是否是发送给当前客户端的数据包
                if (params["Account"].toString() == m_strAccountId
                    || params["Account"].toString() == m_strMoblie) {

                    qDebug() << "接收到服务端的群聊成员数据";
                    addLogGing(QString("接收到服务端的群聊成员数据 cmd：" + QString::number(cmd)));
                    QList<QStringList> list = params["tableData"].value<QList<QStringList>>();
                    const QMap<QString, QByteArray>& map = params["MapData"].value<QMap<QString, QByteArray>>();
                    int rows = params["rows"].toInt();
                    int colunms = params["colunms"].toInt();

                    //返回从服务器接收到的群聊成员数据, 并发送接收成功得信号
                    QueryServerGroup(list, rows, colunms, map);
                   
                }
                break; 
                
            }
            case USERY_SQL_GROUP_USER:      //接收服务端返回的群聊信息数据
            {

                //验证是否是发送给当前客户端的数据包
                if (params["Account"].toString() == m_strAccountId
                    || params["Account"].toString() == m_strMoblie) {

                    QString strAccount = params["Account"].toString();
                     qDebug() << "接收到服务端的群聊信息数据 ";
                     addLogGing(QString("接收到服务端的群聊信息数据 cmd：" + QString::number(cmd)));
                     addServerUserGroup(params);
                   
                    break;
                }
                break;
            }
            case SERVER_GROUP:  //接收群聊信息
            case SERVER_USER:   //接收联系人消息
            {

                //数据的转换
                ChatMsg msg =  QPeratioFile::instance()->QVariantMapToChatMsg(params);

                //判断当前消息数据是否发送给自己的
                if(m_strMoblie == msg.receive){
                    qDebug()<<"接收到群聊消息数据 << cmd:" << cmd;
                    addLogGing(QString("接收到群聊消息数据 cmd：" + QString::number(cmd)));
                    emit QueryGroupMessage(msg);
                }
                break;

            }
            //进行视频通话处理
            case Cmd_VideoCall_Invite:
            case Cmd_VideoCall_UdpAddr:
            {
                qDebug() << "收到视频相关信令，cmd=" << cmd;
                addLogGing(QString("收到视频相关信令 cmd：" + QString::number(cmd)));
                onRecvSignal(params);
                break;
            }

            case   USERY_SQL_INFORMATION:   //服务器返回用户信息
            {

                if(params["Account"].toString() == m_strMoblie){
                    qDebug() <<"接收到用户信息数据 cmd = " << cmd ;
                    addLogGing(QString("接收到用户信息数据  cmd：" + QString::number(cmd)));
                    bool isEmpty = params["isEmpty"].toBool();
                    QString strName = params["strName"].toString();
                    QByteArray ImageData = params["strImage"].toByteArray();
                    QString strSign = params["strSign"].toString();
                    QString Account = params["Account"].toString();
                    QString addAccount = params["addAccount"].toString();

                    emit QUeryUserInformation(isEmpty, strName, ImageData, strSign);
                    break;
                }
            }

            case SEVER_ADDUSER_FRIENDS: //有用户请求添加好友得申请
            {

                if(m_strMoblie == params["Account"].toString()){
                    QString str ="接收到有用户请求添加好友的指令 cmd：" +QString::number(cmd);
                    qDebug()<< str;
                    addLogGing (str);

                    QString strName = params["strName"].toString();
                    QString strSign = params["strSign"].toString();
                    QByteArray ImageData = params["ImageData"].toByteArray();
                    QString Account = params["addAccount"].toString();

                    emit QueryAddUser(strName, strSign, Account, ImageData);
                    break;
                }
            }

            case    SERVER_USER_AGREE: //返回添加同意信令
            {
                qDebug() <<"接收到同意添加好友的指令 cmd：" << cmd;
                if(m_strMoblie == params["addAccount"].toString()){
                    addLogGing(QString("接收到同意添加好友的指令 cmd：" + QString::number(cmd)));
                    qDebug() <<"接收到同意添加好友的指令 cmd：" << cmd;
                    QString strName = params["strName"].toString();
                    QString strSign = params["strSign"].toString();
                    QByteArray ImageData = params["ImageData"].toByteArray();
                    QString Account = params["Account"].toString();

                    emit QueryUserFriendsInformation(strName, strSign, Account, ImageData);
                    break;
                }

            }
            case NEW_SQL_USER:  //放回注册用户成功的信令
            {
                if(m_strMoblie == params["addAccount"].toString()){
                    const QString str = "接收到注册用户的信令, cmd: " + QString::number(cmd);
                    qDebug()<<"接收到注册用户的信令, cmd: " << cmd;
                    const bool isEmpty = params["isEmpty"].toBool();
                    const QString strData = params["strData"].toString();
                    addLogGing(str);
                    emit NewSQLUser(isEmpty, strData);
                    break ;
                }
            }   
            case OFFLINE_NSG_PUSH:   //服务端向客户端推送离线消息数组
            {

                if(m_strMoblie == params["Account"].toString()){

                    if(!params["ifOffline"].toBool()){
                        break;
                    }
                    qDebug()<< "接收到服务器推送的离线消息数组, cmd:" <<QString::number(cmd);
                    const QList<QVariantMap>& Msglist = params["MsgList"].value<QList<QVariantMap>>();
                    QVariantMap params = Msglist[1] ;

                    emit ReceiverOfflineMSg(Msglist);
                    break;
                }
            }
            case GET_OFFLINE_MSG_IMAGE:   //接收服务器下发的图像2进制数据
            {
                if(m_strMoblie == params["Account"].toString()){
                    qDebug()<< "接收到服务器下发的离线图像消息数据，cmd:" <<QString::number(cmd);

                    const QString& ImageName = params["ImageName"].toString();
                    const QByteArray& ImageData = params["ImageData"].toByteArray();
                    //写入到图像消息缓存文件
                    QPeratioFile::instance()->WriteFileImage(QPeratioFile::instance()->getJSONImageURL(),
                                                            ImageName, ImageData);
                }
                break;
            }

            default:
                qDebug() << "客户端收到未知指令 cmd:" << cmd;
                addLogGing(QString("客户端收到未知指令" + QString::number(cmd)));
                break;
        }
	}

}

//连接断开回调
void TcpClient::onDisconnected() {

	qDebug() << "客户端与服务端断开连接";
    m_heartbeatTimer.stop(); // ✅ 停止心跳
	// 清空缓冲区，防止残留脏数据影响下次连接
	m_recvBuf.clear();
}

// 网络错误回调
void TcpClient::onErrorOccurred(QAbstractSocket::SocketError err) {

	//打印错误文本信息
	qDebug() << "客户端错误:" << m_socket.errorString(); 

    //发送连接失败的信号
    emit SuccessServer(false);
}

//发送心跳数据包
void TcpClient::slotSendHeartbeat()
{
    if(m_socket.state() != QTcpSocket::ConnectedState){
        return ; //未连接客户端不发送
    }

    QVariantMap params;
    params["Account"] = m_strMoblie;

    //sendMsg(CMD_HEARTBEAT, params);
}
