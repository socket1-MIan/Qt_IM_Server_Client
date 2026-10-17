#pragma once

# include "BasicWindow.h"
# include "ui_UserLogin.h"
#include "TcpClient.h"
#include "registrationuser.h"
#include "administratorwidget.h"
# include "WindowManager.h"

#include <QObject>
# include <QTimer>

class UserLogin  : public BasicWindow
{
	Q_OBJECT

public:
	UserLogin(BasicWindow* parent = nullptr);
	~UserLogin();

	Ui::Form ui;
private slots:	
	bool veryfyAccountCode(bool& isAccountLogin, QString& strAccount);	//验证账号密码

	void  veryfyAccountServerCode();		//向服务器发送账号密码的验证
	void veryfyAccountLogin(bool in);		//验证登录
	bool connectMySql();					//连接到数据库
	void onLoginBtnClicked();				// 点击按钮
    void onRegistBtnClicked();              //点击注册用户
    void sendRegistBtnClicked();            //发送注册用户的信息

    //接收服务器返回的注册用户结果数据集
    void receiveServerNewUser(const bool isEmpty, const QString strData);

private :
	
	void initControl();			// 初始化控件
	void initTitleBarColor(); 	//设置初始化标题的颜色
    //bool SuccessTCP(bool);		//判断服务器是否连接成功
	void  initTCPServer();		// 连接服务器
protected:
    //键盘事件
    void  keyPressEvent(QKeyEvent *event) override;

private :
    bool m_isLogin;
    RegistrationUser* m_Registration; //注册用户界面
    QTimer* m_TimeSocket;    //当断开与服务器的连接时，顶时请求连接服务器

};

