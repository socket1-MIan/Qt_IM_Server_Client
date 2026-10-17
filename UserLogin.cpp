#include "UserLogin.h"
# include "CCMianWidget.h"
# include "CommonUtils.h"
# include "TcpClient.h"

# include <QMessageBox>
# include <QSqlDatabase>
# include <QSqlQuery>
#include <QSqlError>
# include <QMessageBox>


QString gLoginEmployeeID; //登录者的手机号码

UserLogin::UserLogin(BasicWindow*parent)
	: BasicWindow(parent)
{


	ui.setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->initTitleBar();
	this->setTitleBarTitle("", ":/Resources/MainWindow/qqlogoclassic.png");
	this->loadStyleSheet("UserLogin");
    initControl();
    initTCPServer();

}

UserLogin::~UserLogin()
{


}
//连接到数据库
bool UserLogin::connectMySql()
{

}

//验证账号密码
bool UserLogin::veryfyAccountCode(bool& isAccountLogin, QString& strAccount)
{
    return false;
}

//向服务器发送账号密码的验证
void UserLogin::veryfyAccountServerCode()
{
	QString strAccountInput = ui.editUserAccount->text();
	QString strCodeInput = ui.editPassword->text();
	if (strAccountInput.isEmpty() || strCodeInput.isEmpty()) {
		QMessageBox::information(this, 
			"提示",
			"账号密码不能为空");
		return; 
	}
	TcpClient::getInstance()->veryfyAccountServerCode(strAccountInput, strCodeInput);
}

// 初始化控件
void UserLogin::initControl()
{
    m_Registration = new RegistrationUser(this);

	QLabel* headlabel = new QLabel(this);
	headlabel->setFixedSize(68,68);
	QPixmap pix(":/Resources/MainWindow/head_mask.png"); //空圆头像
	headlabel->setPixmap(getRoundImage(QPixmap(":/Resources/MainWindow/app/logo.ico"),
			pix, headlabel->size()));
	headlabel->move(width() /2 - 34, ui.titleWidget->height() - 34);
	connect(ui.loginBtn, &QPushButton::clicked, this,
		&UserLogin::onLoginBtnClicked);

    //接收服务器账号密码验证成功的信息， 并开始登录
    connect(TcpClient::getInstance(), &TcpClient::QuerySercerCode, this, &UserLogin::veryfyAccountLogin);

    //将点击注册用户按键进行绑定
    connect(ui.registBtn, &QPushButton::clicked, this, &UserLogin::onRegistBtnClicked);
    //将发送注册用户的信号和按键进行绑定
    connect(m_Registration->getPusButton(), &QPushButton::clicked,
                this, &UserLogin::sendRegistBtnClicked);
    //接收服务器返回的注册用户的结果
    connect(TcpClient::getInstance(), &TcpClient::NewSQLUser,
                this, &UserLogin::receiveServerNewUser);

    m_isLogin = false;

}

//设置初始化标题的颜色
void UserLogin::initTitleBarColor()
{
	QColor color = CommonUtils::getDefaultSkinColor(); 

}

// 连接服务器
void UserLogin::initTCPServer()
{

    AdministratorWidget::getInstance()->ConnectionServer();

	//连接是否连接成功的信号
    //connect(TcpClient::getInstance(), &TcpClient::SuccessServer, this, &UserLogin::SuccessTCP);
}

//键盘事件
void UserLogin::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_F3){
        AdministratorWidget::getInstance()->show();
    }
}



// 点击登录按钮
void UserLogin::onLoginBtnClicked() {
	//防止联系点击登录按键，多次向服务器请求数据
	
	//bool isAccountLogin;
	QString strAccount; //账号或者QQ号码
	//发送账号数据验证
	veryfyAccountServerCode();
	
	
}

//点击注册用户
void UserLogin::onRegistBtnClicked()
{
    m_Registration->show();
}

//发送注册用户的信息
void UserLogin::sendRegistBtnClicked()
{
    const QString strAccount = m_Registration->getAccount();
    const QString strName = m_Registration->getName() ;
    if(strAccount.isEmpty() || strName.isEmpty()){
        QMessageBox::information(this, "提示", "输入不能为空");
        return ;
    }

    TcpClient::getInstance()->NewUser(strAccount, strName);
    m_Registration->closeLineEdit();

}

//接收服务器返回的注册用户结果数据集
void UserLogin::receiveServerNewUser(const bool isEmpty, const QString strData)
{
    if(!isEmpty){
        QMessageBox::information(this, "提示", strData);
        return ;
    }else{
         QMessageBox::information(this, "提示", "注册成功");
        m_Registration->close();
         return ;
    }
}

// 验证登录
void UserLogin::veryfyAccountLogin(bool in)
{

	if (in ) {// 判断是否验证成功并开始登录
		const QString strAccountInput = ui.editUserAccount->text();
        gLoginEmployeeID = strAccountInput;

		CCMianWidget* mianWidget = new CCMianWidget(strAccountInput, true);
        mianWidget->show();
        mianWidget->onShowNormal(true);



        //登录成功向服务器发送登录成功的回执
        TcpClient::getInstance()->sendServerLoginSuccess(strAccountInput);
        //拿取当前客户端的手机号和 QQ号
        TcpClient::getInstance()->getServerUserID(strAccountInput);
        close();
	}
	else {
		QMessageBox::information(nullptr, QString::fromUtf8("提示"),
			QString::fromUtf8("输入的账号或者密码有错误,重新输入"));
		return;
	}
}
