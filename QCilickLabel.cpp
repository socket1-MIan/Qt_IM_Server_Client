#include "QCilickLabel.h"

QCilickLabel::QCilickLabel(QWidget *parent)
	: QLabel(parent)
{}

QCilickLabel::~QCilickLabel()
{}
// 按下事件
void QCilickLabel::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton) {
		emit clicked(); 
	}
}

