#pragma once

#include <QObject>

//修改皮肤
class NotifyManager  : public QObject
{
	Q_OBJECT

public:
	NotifyManager();
	~NotifyManager();

signals:
	void signalSkinChanged(const QColor& color);
public:
	static NotifyManager* getInstance();

	//改变皮肤颜色
	void notifyOtherWindowChangeSkin(const QColor& color);

private:
	static NotifyManager* instance; 
};

