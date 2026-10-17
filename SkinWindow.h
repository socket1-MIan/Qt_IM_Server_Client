#pragma once

#include <QWidget>
#include "ui_SkinWindow.h"
# include "basicwindow.h"
#include "QCilickLabel.h"
# include "NotifyManager.h"

class SkinWindow : public BasicWindow
{
	Q_OBJECT

public:
	SkinWindow(QWidget *parent = nullptr);
	~SkinWindow();

	void initControl(); 

public slots:
	void onShowClose(); 


private:
	Ui::SkinWindow ui;
};

