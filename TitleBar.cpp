#include "TitleBar.h"

# define BUTTON_HEIGHT 27	//按钮的高度
# define BUTTON_WIDTH  27	//按钮的宽度
# define TITLE_HEIGHT  27	//标题栏的高度

TitleBar::TitleBar(QWidget* parent)
	: QWidget(parent)
	, m_isPressed(false)
	, m_buttonType(MIN_MAX_BUTTON)
{
	initControl();
	initConnections();
	loadStyleSheet("Title");
}

TitleBar::~TitleBar()
{}
// 初始化控件
void TitleBar::initControl() {

	m_pIcon = new QLabel(this);
	m_pTitleContent = new QLabel(this);

	m_pButtonMin = new QPushButton(this);
	m_pButtonRestore = new QPushButton(this);
	m_pButtonMax = new QPushButton(this);
	m_pButtonClose = new QPushButton(this);

	m_pButtonMin->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
	m_pButtonRestore->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
	m_pButtonMax->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
	m_pButtonClose->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));

	// 设置对象名
	m_pTitleContent->setObjectName("TitleContent");
	m_pButtonMin->setObjectName("ButtonMin");
	m_pButtonRestore->setObjectName("ButtonRestore");
	m_pButtonMax->setObjectName("ButtonMax");
	m_pButtonClose->setObjectName("ButtonClose");

	// 设置布局
	QHBoxLayout* mylayout = new QHBoxLayout(this);
	mylayout->addWidget(m_pIcon);
	mylayout->addWidget(m_pTitleContent);

	mylayout->addWidget(m_pButtonMin);
	mylayout->addWidget(m_pButtonRestore);
	mylayout->addWidget(m_pButtonMax);
	mylayout->addWidget(m_pButtonClose);

	mylayout->setContentsMargins(5, 0, 0, 0);
	mylayout->setSpacing(0);
	
	// 设置标题栏的尺寸策略
	m_pTitleContent ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setFixedHeight(TITLE_HEIGHT);
	this->setWindowFlags(Qt::FramelessWindowHint);

}

// 信号与槽的连接
void TitleBar::initConnections() {

	connect(m_pButtonMin, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
	connect(m_pButtonRestore, SIGNAL(clicked()), this, SLOT(onButtonRestoeClicked()));
	connect(m_pButtonMax, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
	connect(m_pButtonClose, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
	

}

// 设置标题栏的图标
void TitleBar::setTitleIcon(const QString& filePath) {

	QPixmap titleIcon(filePath);
	m_pIcon->setFixedSize(titleIcon.size());
	m_pIcon->setPixmap(titleIcon);

}

//设置标题栏的内容
void TitleBar::setTitleContent(const QString& titleContent) {

	m_pTitleContent->setText(titleContent);
	m_titleContent = titleContent; 

}

// 设置标题栏的长度
void TitleBar::setTitleWidth(int width) {

	this->setFixedHeight(width);
}

// 设标题栏的按钮 类型 
void TitleBar::setButtonType(ButtonType buttontype) {

	m_buttonType = buttontype; 

	switch (buttontype)
	{
	case MIN_BUTTON:{
		m_pButtonRestore->setVisible(false); //设置不可见
		m_pButtonMax->setVisible(false);
	}
		break;
	case MIN_MAX_BUTTON: {
		m_pButtonRestore->setVisible(false);
	}
		break;
	case ONLY_CLOSE_BUTTON: {
		m_pButtonRestore->setVisible(false);
		m_pButtonClose->setVisible(false);
		m_pButtonMin->setVisible(false); 
	}
		break;
	default:
		break;
	}
}

// 保存窗口最大化前窗口的位置以及大小 
void TitleBar::saveRestoreInfo(const QPoint& point, const QSize& size) {

	m_restorPos = point;
	m_restoreSize = size; 
}

// 获取窗口最大化前窗口的位置以及大小
void TitleBar::getRestoreInfo(QPoint& point, QSize& size) {
	point = m_restorPos;
	size = m_restoreSize;
}

// 绘制标题栏
void TitleBar::paintEvent(QPaintEvent* event) {

	// 设置背景色
	QPainter painter(this);
	QPainterPath pathBack;
	pathBack.setFillRule(Qt::WindingFill); //设置填充规则
	pathBack.addRoundedRect(QRect(0, 0, width(), height()), 3, 3); //添加圆角矩形到绘图路径
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	//当窗口最大或还原后， 窗口长度改变， 标题栏相应做出改变
	//parentWidget() 返回夫部件

    if (parentWidget() != nullptr && (width() != parentWidget()->width())) {
		setFixedWidth(parentWidget()->width());
	}
	QWidget::paintEvent(event);
}

//双击事件， 实现双击标题栏的最大化和最小化
void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {

	// 只有存在最大化和最小化按钮才存在
	if (m_buttonType == MIN_MAX_BUTTON) {
		if (m_pButtonMax->isVisible())
			onButtonMaxClicked();
		else
			onButtonRestoeClicked(); 
	}
	return QWidget::mouseDoubleClickEvent(event);
}

// 通过鼠标按下， 鼠标移动， 鼠标释放事件来达到标题栏移动效果
// 鼠标按下事件 
void TitleBar::mousePressEvent(QMouseEvent* event) {

	if (m_buttonType == MIN_MAX_BUTTON) {
		//窗口最大化时禁止拖动
		if (m_pButtonMax->isVisible()) {
			m_isPressed = true;
			m_startMovePos = event->globalPosition().toPoint();
		}
	}
	else {
		m_isPressed = true;
		m_startMovePos = event->globalPosition().toPoint();
	}
	 QWidget::mousePressEvent(event);
}

//鼠标移动事件
void TitleBar::mouseMoveEvent(QMouseEvent* event) {
	if (m_isPressed) {
        QWidget* parent = parentWidget();
        if(!parent)
        {
            QWidget::mouseMoveEvent(event);
            return;
        }
		QPoint movePoint = event->globalPosition().toPoint() - m_startMovePos;
		QPoint widgetPos = parentWidget()->pos();
		m_startMovePos = event->globalPosition().toPoint();
		parentWidget()->move(widgetPos.x() + movePoint.x(),
			widgetPos.y() + movePoint.y());
	}
	QWidget::mouseMoveEvent(event);
}

//鼠标释放事件 
void TitleBar::mouseReleaseEvent(QMouseEvent* event) {

	m_isPressed = false;
	QWidget::mouseReleaseEvent(event);
}

// 加载样式表
void TitleBar::loadStyleSheet(const QString& sheeetName) {

	QFile file(":/Resources/QSS/" + sheeetName +".css");
	file.open(QFile::ReadOnly);
	if (file.isOpen()) {

		QString styleSheet = this->styleSheet();
		styleSheet += QLatin1String(file.readAll());
		this->setStyleSheet(styleSheet);
	}
	else {
        qDebug() << "在TitleBar 加载样式表 QFile.open()打开失败";
	}
}

// 最小化按钮
void TitleBar::onButtonMinClicked() {
	emit signalButtonMiaClicked();
}

// 最大化还原按钮
void TitleBar::onButtonRestoeClicked() {
	m_pButtonRestore->setVisible(false);
	m_pButtonMax->setVisible(true);
	emit signalButtonMaxClickded();
}

//最大化按钮
void TitleBar::onButtonMaxClicked() {

	m_pButtonMax->setVisible(false);
	m_pButtonRestore->setVisible(true);
	emit signalButtonMaxClickded(); 
}

// 关闭按钮
void  TitleBar::onButtonCloseClicked() {
	emit signalButtonCloseClicked(); 
}
