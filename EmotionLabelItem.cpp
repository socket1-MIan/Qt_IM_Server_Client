#include "EmotionLabelItem.h"

EmotionLabelItem::EmotionLabelItem(QWidget *parent)
	: QCilickLabel(parent)
{
	initControl(); 
	connect(this, &QCilickLabel::clicked, this, [=]{
		emit emotionClicked(m_emotionNamel);
		});
}

EmotionLabelItem::~EmotionLabelItem()
{}
//设置表情得序号
void EmotionLabelItem::setEmotionName(int emotionName)
{
	m_emotionNamel = emotionName;
	QString inageName = QString(":/Resources/MainWindow/emotion/%1.png").arg(m_emotionNamel);
	m_apngMovie = new QMovie(inageName, "apng", this);
	m_apngMovie->start();
	m_apngMovie->stop();
	setMovie(m_apngMovie);
	
}

void EmotionLabelItem::initControl()
{
	setAlignment(Qt::AlignCenter);	//居中对齐
	setObjectName("emotionLabelItem");
	setFixedSize(32,32);

}

