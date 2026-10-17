#ifndef REGISTRATIONUSER_H
#define REGISTRATIONUSER_H

#include "basicwindow.h"
#include <QPushButton>

namespace Ui {
class RegistrationUser;   // 前向声明即可
}

class RegistrationUser : public BasicWindow
{
    Q_OBJECT
public:
    explicit RegistrationUser(QWidget *parent = nullptr);
    ~RegistrationUser();

    QString getAccount();
    QString getName();
    QPushButton* getPusButton() const;
    void closeLineEdit();   //清空输入的结果集

private:
    Ui::RegistrationUser *ui;   // 指针，前向声明足够
};

#endif
