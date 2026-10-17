#pragma once

#include <QLabel>
# include<QMouseEvent>


class QCilickLabel  : public QLabel
{
	Q_OBJECT

public:
	QCilickLabel(QWidget *parent);
	~QCilickLabel();

protected:
	// 按下事件
	void mousePressEvent(QMouseEvent* event)override;
signals:
	void clicked(); 

};

