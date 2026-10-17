#include "basicwindow.h"
# include "CommonUtils.h"
# include "NotifyManager.h"

# include <QSqlDatabase>
# include <QSqlQuery>
// 外部全局变量，登陆者ID
extern QString gLoginEmployeeID; 

// 窗口的基类
BasicWindow::BasicWindow(QWidget *parent)
	: QDialog(parent)
{
	// 获取默认的颜色值
    m_colorBackGround = CommonUtils::getDefaultSkinColor();

    this->setWindowFlags(Qt::FramelessWindowHint);// 无边框
	this->setAttribute(Qt::WA_TranslucentBackground, true);//透明
	// 发射更改皮肤信号，修改皮肤
	connect(NotifyManager::getInstance(), SIGNAL(signalSkinChanged(const QColor&)),
		this,SLOT(onSignalSkinChanged(const QColor&)));
}

BasicWindow::~BasicWindow()
{

}

//皮肤改变
void BasicWindow::onSignalSkinChanged(const QColor& color) {

	m_colorBackGround = color;
	loadStyleSheet(m_styleName);
}

//设置标题栏的Lgo和标题
void BasicWindow::setTitleBarTitle(const QString& title, const QString& icon) {

	_titleBar->setTitleIcon(icon);
	_titleBar->setTitleContent(title);
}

//初始化标题栏
void BasicWindow::initTitleBar(ButtonType buttontype) {

	//载入标题栏
	_titleBar = new TitleBar(this);
	_titleBar->setButtonType(buttontype);
	_titleBar->move(0,0);

	connect(_titleBar, &TitleBar::signalButtonMiaClicked,
		this, &BasicWindow::onButtonMinClicked);
	connect(_titleBar, &TitleBar::signalButtonRestoreClicked, 
			this, &BasicWindow::onButtonRestoreCliced);
	connect(_titleBar, &TitleBar::signalButtonMaxClickded,
			this, &BasicWindow::onButtonMaxClicked);
	connect(_titleBar, &TitleBar::signalButtonCloseClicked, 
			this, &BasicWindow::onButtonCloseClicked);

}

// 加载样式表 
void BasicWindow::loadStyleSheet(const QString& sheetName) {

	m_styleName = sheetName;
	QFile file(":/Resources/QSS/" + sheetName + ".css");

	file.open(QFile::ReadOnly);

	if (file.isOpen()) {

		this->setStyleSheet("");
		QString qsstyleSheet = QLatin1String(file.readAll());

		//获取用户当前的皮肤的RGB值
		QString r = QString::number(m_colorBackGround.red());
		QString g = QString::number(m_colorBackGround.green());
		QString b = QString::number(m_colorBackGround.blue());

		qsstyleSheet += QString("QWidget[titleskin=true]\
								{background-color:rgb(%1,%2,%3);\
								border-top-left-radius:4px;}\
								QWidget[bottomskin=true]\
								{border-top:1px solid rgba(%1,%2,%3,100);\
								background-color:rgba(%1,%2,%3,50);\
								border-bottom-left-radius:4px;\
								border-bottom-right-radius:4px;}")
								.arg(r).arg(g).arg(b);
		this->setStyleSheet(qsstyleSheet);
	}
	else {
		qDebug() << "在basicWindow 81行 QFile.open()打开失败";
	}
	file.close();
}

//初始化背景
void BasicWindow::initBackGroundColor() {

	QStyleOption opt;
	opt.initFrom(this);

	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
// 子类化事件时需要重写绘图事件设置背景图
//绘制事件
void BasicWindow::paintEvent(QPaintEvent* event) {

	initBackGroundColor(); 
	QDialog::paintEvent(event);

}

// 获取圆头像
// src , 来源  ,   mask , 背景, 一个空的圆头像  ,   maskSize, 尺寸大小
QPixmap BasicWindow::getRoundImage(const QPixmap& src, QPixmap& mask, QSize maskSize) {

	if (maskSize == QSize(0,0)) {
		maskSize == mask.size();
	}
	else {
		mask = mask.scaled(maskSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		if (mask.isNull()) {
			qDebug() << "basicWindow 113行 Pixmap 为空";
		}
	}
	//保存转换后的头像
	QImage resultIamge(maskSize, QImage::Format_ARGB32_Premultiplied);
	QPainter painter(&resultIamge);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultIamge.rect(), Qt::transparent);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawPixmap(0, 0, mask);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawPixmap(0, 0, src.scaled(maskSize,Qt::KeepAspectRatio, Qt::SmoothTransformation));
	painter.end(); 

	return QPixmap::fromImage(resultIamge);
}

//窗口基类
void BasicWindow::onShowClose(bool) {
	close();
}

void BasicWindow::onShowMin(bool) {
	showMinimized();
}

void BasicWindow::onShowHide(bool) {
	
	hide();
}

void BasicWindow::onShowNormal(bool) {

	show();
	activateWindow();
}

void BasicWindow::onShowQuit(bool) {
	//更新登录状态 为离线

    emit execProgram();
	QApplication::quit(); 
}

// 鼠标移动事件
void BasicWindow::mouseMoveEvent(QMouseEvent* event) {

	if (m_mousePressed && (event->buttons() && Qt::LeftButton)) {

		move(event->globalPosition().toPoint() - m_mousePoint);
		event->accept();
	}

}

//鼠标按下事件 
void BasicWindow::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		// pos() , 事件发生时, 相对于窗口左上角的 偏移坐标
		m_mousePressed = true;
		m_mousePoint = event->globalPosition().toPoint() - pos();// 减去 坐标值
		event->accept();// 接受
	}
}

//鼠标释放事件 
void BasicWindow::mouseReleaseEvent(QMouseEvent* event) {
	m_mousePressed = false; 

}

// 最小化按钮按下
void BasicWindow::onButtonMinClicked() {

	if (Qt::Tool == (windowFlags() & Qt::Tool)) {
		hide(); 
	}
	else {
		showMinimized(); 
	}
}

//最大化按钮
void BasicWindow::onButtonMaxClicked() {

	_titleBar->saveRestoreInfo(pos(), QSize(width(), height()));
	//获取屏幕的几何信息
	//获取主屏幕的可用几何区域（即屏幕尺寸减去任务栏、系统托盘等占据的区域）
	QRect desktopRect = QGuiApplication::primaryScreen()->availableGeometry();
	QRect factRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3,
		desktopRect.width() + 6, desktopRect.height() + 6);
	setGeometry(factRect);

}

//最大化以及还原按钮 
void BasicWindow::onButtonRestoreCliced() {

	QPoint windowPos;
	QSize windowSize; 
	_titleBar->getRestoreInfo(windowPos, windowSize);
	setGeometry(QRect(windowPos, windowSize));

}

// 关闭按钮 
void BasicWindow::onButtonCloseClicked() {

	close(); 
}
