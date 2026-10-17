#pragma once

#include <QWidget>
# include <QLabel>
# include <QPushButton>
# include <QHBoxLayout>
# include <QPainter>
# include <QPainterPath>
# include <QMouseEvent>
# include <QFile>

enum ButtonType {

	MIN_BUTTON = 0, //最小化以及关闭按钮
	MIN_MAX_BUTTON , // 最小化， 最大化以及关闭按钮
	ONLY_CLOSE_BUTTON //只有关闭按钮
};

// 自定义标题栏
class TitleBar  : public QWidget
{
	Q_OBJECT

public:
	TitleBar(QWidget *parent = nullptr);
	~TitleBar();

	void setTitleIcon(const QString& filePath);		// 设置标题栏图标
	void setTitleContent(const QString& titleContent);// 设置标题栏内容
	void setTitleWidth(int width);				// 设置标题栏长度
	void setButtonType(ButtonType buttonType);	// 设置标题栏按钮类型

	// 保存, 获取窗口最大化前的窗口的位置以及大小
	void saveRestoreInfo(const QPoint& point, const QSize& size);
	void getRestoreInfo(QPoint& point, QSize& size);

private :
	void paintEvent(QPaintEvent* event)override;			//绘制事件
	void mouseDoubleClickEvent(QMouseEvent* event)override;	// 鼠标双击事件
	void mousePressEvent(QMouseEvent* event)override;		//鼠标按下事件
	void mouseMoveEvent(QMouseEvent* event)override;		//鼠标移动事件
	void mouseReleaseEvent(QMouseEvent* event)override;		//鼠标松开事件

	void initControl();		//初始化控件
	void initConnections(); //初始化信号与槽的链接
	void loadStyleSheet(const QString& sheetName); //加载样式表

signals:
	// 标题栏按钮点击时触发的信号
	void signalButtonMiaClicked();		//最小化按钮
	void signalButtonRestoreClicked();  //最大化还原按钮
	void signalButtonMaxClickded();		//最大化按钮
	void signalButtonCloseClicked();	//关闭按钮

private slots:
	//按钮响应的槽
	void onButtonMinClicked();
	void onButtonRestoeClicked();
	void onButtonMaxClicked();
	void onButtonCloseClicked();

private:
	QLabel* m_pIcon;			//标题栏的图标
	QLabel* m_pTitleContent;	//标题栏的内容
	QPushButton* m_pButtonMin;  //最小化的按钮 
	QPushButton* m_pButtonRestore; //最小化还原按钮 
	QPushButton* m_pButtonMax;;		//最大化按钮
	QPushButton* m_pButtonClose;	//关闭按钮 

	//最大化还原按钮变量(用于保存窗口位置以及大小) 
	QPoint m_restorPos;
	QSize m_restoreSize; 

	// 移动窗口变量
	bool m_isPressed; 
	QPoint m_startMovePos; 

	QString m_titleContent;		//标题栏内容
	ButtonType m_buttonType;	//标题栏右上角的按钮类型

};

