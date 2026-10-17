#pragma once

#include <QWidget>

#include "ui_ReceiveFile.h"
# include "basicwindow.h"

//接收文件
class ReceiveFile : public BasicWindow
{
	Q_OBJECT

public:
	ReceiveFile(QWidget *parent = nullptr);
	~ReceiveFile();
	//设置的信息
	void setMsg(QString& msgLabel);

signals:
	//拒绝接收文件
	void refuseFile();

private slots:
	//取消接收文件
	void on_canelBtn_clicked();
	//确认接收文件
	void on_okBtn_clicked(); 
private:
	Ui::ReceiveFileClass ui;
};

