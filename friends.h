#ifndef FRIENDS_H
#define FRIENDS_H

#include <QDialog>
# include "basicwindow.h"

# include <QPushButton>

namespace Ui {
class Friends;
}

class Friends : public QDialog
{
    Q_OBJECT

public:
    explicit Friends(QWidget *parent = nullptr);
    ~Friends();
    QPushButton* getAddBtn();
    QLabel* getBel();
    QLabel* getBelAccount();
    QLabel* getBelName()const;
    QPixmap getLabImage()const;        //拿取用户的头像
    QString getLabSignStr() const ;     //拿取个性签名



    void setUIWidget(QString strName, QPixmap pixmap, QString strSign);   //设置界面
    void setLabSign(const QString& Sign);      //设置个性签名

private:
    Ui::Friends *ui;
    QString LabSign;    //保存的用户签名

};

#endif // FRIENDS_H
