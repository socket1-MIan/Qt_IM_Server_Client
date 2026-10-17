#pragma once
#ifndef BASICWINDOW_H
#define BASICWINDOW_H


#include <QDialog>
# include "TitleBar.h"
# include <QMouseEvent>
# include <QFile>
# include <QStyleOption>
# include <QPainter>
# include <QApplication>
# include <QGuiApplication>
# include <QTimer>

class BasicWindow  : public QDialog
{
	Q_OBJECT

public:
	BasicWindow(QWidget *parent = nullptr);
	virtual~BasicWindow();

public:
	// 加载样式表
	void loadStyleSheet(const QString& sheetName);

	//获取圆头像
	QPixmap getRoundImage(const QPixmap& src, QPixmap& mask, 
		QSize masksize = QSize(0,0));

private:
	void initBackGroundColor(); //初始化背景

protected:
	void paintEvent(QPaintEvent* event)override;		//绘制事件
	void mousePressEvent(QMouseEvent* event)override;	//鼠标事件
	void mouseMoveEvent(QMouseEvent* event)override;	//鼠标移动事件
	void mouseReleaseEvent(QMouseEvent* event)override; //鼠标移动事件

protected:
	void initTitleBar(ButtonType buttontype = MIN_BUTTON);
	void setTitleBarTitle(const QString& title, const QString& icon="");

public slots:
	void onShowClose(bool);		//显示关闭
	void onShowMin(bool);		//最小化
	void onShowHide(bool);		//窗口进行隐藏
	void onShowNormal(bool);	//窗口常态
	void onShowQuit(bool);		//推出
	void onSignalSkinChanged(const QColor& color);		//更改皮肤

	void onButtonMinClicked();		//最小化按钮
	void onButtonRestoreCliced();	//还原按钮
	void onButtonMaxClicked();		//最大化按钮
	void onButtonCloseClicked();	//关闭按钮

signals:
    // 发送退出程序的信号
    void execProgram();

public:
	QPoint m_mousePoint;	//鼠标位置
	bool m_mousePressed;	//鼠标是否按下
	QColor m_colorBackGround; //背景色
	QString m_styleName;	//样式文件名称
	TitleBar* _titleBar;	//标题栏
};

// ... 类定义
#endif
