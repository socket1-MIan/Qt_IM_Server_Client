#include "registrationuser.h"
#include "ui_registrationuser.h"

RegistrationUser::RegistrationUser(QWidget *parent)
    : BasicWindow(parent)
    , ui(new Ui::RegistrationUser)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    initTitleBar();                    // ✅ 必须，否则没有标题栏
    setTitleBarTitle("注册账号", "");
    //setFixedSize(280, 180);                        // ✅ 强制设置尺寸，和 ui 文件一致
    initTitleBar(ButtonType::ONLY_CLOSE_BUTTON);
    setWindowFlags(windowFlags() | Qt::Dialog);  // ✅ 加这行，恢复 Dialog 标志
    setStyleSheet("background-color:rgb(245,245,245);");
    ui->Account->setPlaceholderText("手机号: ");
    ui->Name->setPlaceholderText("QQ号: ");

}

RegistrationUser::~RegistrationUser()
{
    delete ui;
}

//获取用户输入的手机号
QString RegistrationUser::getAccount()
{
    return ui->Account->text();

}

//获取用户输入的qq号
QString RegistrationUser::getName()
{
    return ui->Name->text();

}

// 获取按键
QPushButton* RegistrationUser::getPusButton() const
{
    return ui->pushButton;
}

//清空输入的结果集
void RegistrationUser::closeLineEdit()
{

    ui->Account->clear();
    ui->Name->clear();
}
