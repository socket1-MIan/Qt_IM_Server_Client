#pragma once

# include <QObject>
# include "ChatMsg.h"
# include "videocommdef.h"
# include "videorenderwidget.h"
# include "videocalludp.h"
#include "videowidget.h"
# include "administratorwidget.h"

# include <QTcpSocket>
# include <QByteArray>
# include <QVariantMap>
# include <QImage>
#include <QMetaType>
# include <QByteArray>
# include <QDialog>
# include <QVBoxLayout>
# include <QImage>



# define RETURN_SQL_ACCOUNT         0x0101 //查询验证成功
# define USERY_SQL_QQACCOUNT_USER   0x0102 //客户端QQ号查询验证数据库中的密码
# define USERY_SQL_IAMGE_USER       0x0103 //查询并返回客户端的头像图像数据
# define USERY_SQL_ACCOUNTID        0x0104 //查询并且返回客户端的手机和 QQ号数据
# define ERROR_SQL_ACCOUNT          0x0200 //手机查询数据中的数据失败
# define UERY_SQL_ACCOUNT_USER      0x0202 //客户端手机号查询验证数据库的密码
# define USERY_SQL_GROUP            0x0302 //客户端返回所有群成员信息
# define USERY_SQL_GROUP_USER       0x0303 //客户端返回群信息数据
# define USERY_SQL_INFORMATION      0x0304 //请求返回用户的信息
# define SERVER_GROUP               0x0400 //向群聊发送/接收消息
# define SERVER_USER                0x0401 //向指定联系人发送/接收消息
# define SERVER_USER_AGREE		    0x0502 //向客户端返还添加好友同意指令
# define SEVER_ADDUSER_FRIENDS      0x0501 //向服务器发送添加好友申请
# define SEVER_FRIENDS_FAILURE	    0x0500 //向客户端返还添加失效的指令
# define NEW_SQL_USER               0x0600 //注册用户
# define OFFLINE_NSG_PUSH			0x0701 //客户端请求拉取离线消息
# define MSG_READ_RECEIPT			0x0702 //客户端先服务端发送已读回执
# define GET_OFFLINE_MSG_IMAGE      0x0703 //客户端向服务端2次拉取图像2进制数据
# define GET_OFFLINE_MSG_SUCC		0x0704 //客户端成功拿到离线消息的回执
# define LOGN_SERVER_SUCESS			0x0801 //客户端先服务端发登录成功回执
# define CMD_HEARTBEAT              0x0901 //心跳上报



class TcpClient :  public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);

    //连接服务端
    void connectToServer(const QString& ip, quint16 port);
    //发送协议包
    void sendMsg(uint16_t cmd, const QVariantMap& params);

    void initSignalBind();  //初始化信号连接

    //主动断开
    void disconnectServer();

    //当前是否连接上服务器
    bool ServerSuccess();

    //向服务端验证登录
    void veryfyAccountServerCode(const QString strAccount, const QString strCode);

    //向服务端发送返回客户端手机和 QQ号的请求
    void getServerUserID(const QString strId);

    //向服务端发送登录成功的回执
    void sendServerLoginSuccess(const QString& strAccount);

    //向服务端发送查询验证并返回的图像数据
    void getServerUserImage( QString strAccountInput);

    //向服务端发送请求群成员用户数据
    void getServerUserGroup(const QString strAccountInput);

    //向服务端发送请求群聊信息数据
    void getServerUserGroupID(const QString& strGroupInput, const QString& strAccountInput);

    //接收来自服务端的群聊信息数据
    void addServerUserGroup(const QVariantMap& params);

    //向指定的群聊发送消息数据
    void SendGroupId(const ChatMsg& msg);

    //向指定联系人发送消息数据
    void SendUser(const ChatMsg& msg);

    //向服务器发送添加好友的申请
    void addServerUserFriends (const QString &addAccount, const QString &Account);

    //向服务器请求用户的个人信息
    void getServerUserInformation(const QString& Account, const QString& addAccount);

    //向服务器发送同意添加好友得申请
    void addArreeAddUser(const QString& Account, const QString& addAccount);

    //新键用户
    void NewUser(const QString& Accoutn, const QString& strName);

    //向服务端请求离线消息数据
    void getOfflineMsg(const QString& Account);

    //用户拉取到离线消息向服务器发送回执
    void sendServerOfflineMagMuccess(const QString& Account);

    //用户向客户端发送消息已读回执
    void sendServerReanMsg(const QString& sendAccount, const QString& recerAccount);

    //向服务端2次拉取图像离线消息2进制数据
    void getServerOfflineMsgImaeg(const QString& Account, const QString& ImageName);

    /********  工具函数  *********/

    //将图像数转换为2进制
    QByteArray pixmapToBytes(const QPixmap& pix);

    //追加到日志界面
    void addLogGing(const QString& strData);


    // *********音视频*********//
    void initVideWidget();  //初始化视频通话部件
    void initVideWidget(QWidget* widget, QVBoxLayout* VBoxLayout); //初始化视频通话部件
    void slotCall();        //主动发起通话呼叫
    void onRecvSignal(const QVariantMap &map);    //接收到通话请求
    void slotCall(QString Account);               //主动发起通话呼叫
    void sotpCall();        //挂断通话
    void receiveCall(const QString fromUser);     //接收到通话状态请求
    //接收到对方ip端口推送
    void receiveIpPort(const QString fromUser, const QString payload);

   // 返回单列对象
    static TcpClient* getInstance();
private slots:
    //连接成功
    void onConnected();
    //接收到数据
    void onReadyRead();
    //断开连接
    void onDisconnected();
    // 错误处理
    void onErrorOccurred(QAbstractSocket::SocketError err);
    //发送心跳数据包
    void slotSendHeartbeat();

signals:
    //返回服务器是否连接成功
    void SuccessServer(bool);

    //返回向服务端的数据查询验证是否成功
    void QuerySercerCode(bool);

    //返回服务端发回的客户端的手机QQ号数据
    void QUeryServerAccount(QMap<QString,QString> map, QString strGroupId, QString strSign);

    //返回从服务器接收到的图像数据
    void QueryServerImage(QImage);

    //返回从服务器接收到的群聊成员数据
    void QueryServerGroup(QList<QStringList>, int, int, const QMap<QString, QByteArray>&);

    //返回从服务端接收到的群聊信息数据
    void QueryServerGroupID(const QString& strNamem,
                            const QString& strSign,
                            const QString& Account,
                            const QImage& image);

    //返回从服务端接收到的已经加入和添加的联系人或者群聊列表
    void QueryServerContact(QString strData, const QVariantMap params);

    //返回 接收的群聊消息数据
    void QueryGroupMessage(const ChatMsg& mag);

    //返回 接收到的用户信息数据
    void QUeryUserInformation(const bool& isEmpty,
                              const QString& strName,
                              const QByteArray& ImageData,
                              const QString& strSign);

    //返回添加的好友信息
    void QueryUserFriendsInformation(const QString& strNamem,
                                     const QString& strSign,
                                     const QString& Account,
                                     const QByteArray& imageData);
    //返回有用户添加自己为好友得请求
    void QueryAddUser(const QString& strNamem,
                      const QString& strSign,
                      const QString& Account,
                      const QByteArray& imageData);

    //发送显示窗口信号
    void sinVideShow();
    //发送隐藏窗口信号
    void sinVideHide();

    //返回注册用户的信号
    void NewSQLUser(const bool isEmpty, const QString strData);

    //返回接收到的 离线消息数组
    void ReceiverOfflineMSg(const QList<QVariantMap>& Msglist);

private:
    QTcpSocket m_socket;
    QByteArray m_recvBuf; // 接收缓冲区，处理粘包半包

    //当前客户端登录的账号
    QString m_strAccountInput;  //当前临时输入的账号信息还未进行服务端账号数据验证
    QByteArray m_ImageUser;     //当前客户端的头像2进制数据
    QMap<QString, QString> m_strAccountID;  //手机号qq号的数据映射
    QString m_strAccountId;     //QQ号
    QString m_strMoblie;        //手机号

    //****音视频*****//
    VideoCallUdp* m_videoUdp ;
    VideoRenderWidget* m_remoteView ;   //远端对方画面
    VideoRenderWidget*  m_localView; //本地自己画面
    QString m_localAccount;
    CallState m_callState = Call_Idle;  //通话状态机
    QString m_peerAccount;  // 当前通话对方账号
    QString m_socketIP;     // 当前通话对方ip
    quint16 m_prot;         // 当前通话对方端口

    QDialog* VideoWidget ; //视频窗口

    //心跳机制
    QTimer m_heartbeatTimer;
    static const int HEARTBEAT_INTERVAL_MS = 25 * 1000; // 25秒发一次

};

