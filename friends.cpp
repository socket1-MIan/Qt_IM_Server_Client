#include "friends.h"
#include "ui_friends.h"

Friends::Friends(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Friends)
{

    ui->setupUi(this);
}

Friends::~Friends()
{
    delete ui;
}

QPushButton *Friends::getAddBtn()
{
    return ui->AddBtn;
}

QLabel *Friends::getBel()
{
    return ui->ImageLabel;
}


QLabel *Friends::getBelAccount()
{
    return ui->AccountLabel;
}

QLabel *Friends::getBelName() const
{

    return ui->NameLabel;
}

QPixmap Friends::getLabImage() const
{

    return ui->ImageLabel->pixmap();
}

//拿取个性签名
QString Friends::getLabSignStr() const
{

    return LabSign ;

}


//设置界面
void Friends::setUIWidget(QString strName, QPixmap pixmap, QString strSign)
{

    ui->NameLabel->setText(strName);
    ui->AccountLabel->setText(strSign);

    ui->ImageLabel->setPixmap(pixmap);


}

//设置个性签名
void Friends::setLabSign(const QString &Sign)
{

    LabSign = Sign;
}
