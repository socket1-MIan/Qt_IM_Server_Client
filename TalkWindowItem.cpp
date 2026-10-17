#include "TalkWindowItem.h"
# include "CommonUtils.h"

TalkWindowItem::TalkWindowItem(QWidget *parent, QString uid)
	: QWidget(parent),
	m_uid(uid)
{
	ui.setupUi(this);
	initControl();

}

TalkWindowItem::~TalkWindowItem()
{}
//设置头像
void TalkWindowItem::setHeadPixmap(const QPixmap& pixmap)
{

	if (pixmap.isNull()) {
		qDebug() << "设置列表index 左侧列表项 头像失败";
		return;
	}

	QPixmap mask = QPixmap(":/Resources/MainWindow/head_mask.png");
	//QPixmap head = QPixmap(":/Resources/MainWindow/girl.png");

	const QPixmap& headpixmap = CommonUtils::getRoundIamge(pixmap, mask, ui.headlabel->size());
	ui.headlabel->setPixmap(headpixmap);

}
//设置内容
void TalkWindowItem::setMsgLabelContent(const QString& msg)
{

	ui.msgLabel->setText(msg);

}
//返回文本
QString TalkWindowItem::getMsgLabelText()
{
	return ui.msgLabel->text();
}

//拿取当前的id
QString TalkWindowItem::getItemUid()
{
	return m_uid; 
}

//设置初始化数据信息
void TalkWindowItem::setinitIndex(const QString& Name, const QString& imageURL)
{

	if (Name.isEmpty() || imageURL.isEmpty()) {
		qDebug() << "设置列表index 左侧列表项时 失败";
		return;
	}

	QPixmap pix(imageURL);
	if (pix.isNull()) {
		qDebug() << "index 左侧列表项 头像数据为空";
		return;
	}
	//设置头像
	setHeadPixmap(pix);

	//设置内容
	setMsgLabelContent(Name); 


}

void TalkWindowItem::initControl()
{
	ui.tclosebtn->setVisible(false);
	connect(ui.tclosebtn, &QPushButton::clicked, [=]() {
		emit signalCloseClicked(m_uid);
		});

}
//进入事件
void TalkWindowItem::enterEvent(QEnterEvent* event)
{
	ui.tclosebtn->setVisible(true);
    QWidget::enterEvent(event);

}
void TalkWindowItem::leaveEvent(QEvent* event)
{
}
//离开事件
void TalkWindowItem::resizeEvent(QResizeEvent* event)
{
	ui.tclosebtn->setVisible(false);
	QWidget::resizeEvent(event);


}

