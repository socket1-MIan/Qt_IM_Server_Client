#pragma once
#include "QCilickLabel.h"
# include <QMovie>

class EmotionLabelItem  : public QCilickLabel
{
	Q_OBJECT

public:
	EmotionLabelItem(QWidget *parent = nullptr);
	~EmotionLabelItem();
	//设置表情得序号
	void setEmotionName(int emotionName);

private :
	void initControl();

signals:
	void emotionClicked(int emotionNum);

private :
	int m_emotionNamel;
	QMovie* m_apngMovie;
};

