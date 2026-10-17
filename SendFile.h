#pragma once

#include <QWidget>
# include <QHostAddress>

# include "basicwindow.h"
#include "ui_SendFile.h"

//发送文件
class SendFile : public BasicWindow
{
	Q_OBJECT

public:
	SendFile(QWidget *parent = nullptr);
	~SendFile();

signals:
	//发送文件发射的信号
	void sendFileClicked(QString& strData, int &msgType, QString fileName);

private slots:
	//打开文件
	void on_openBtn_clicked();
	//发送文件
	void on_sendBtn_clicked();


public:
	Ui::SendFileClass ui;

	//保存用户选择的文件路径 
	QString m_filePath; 
private :
	
};

