#pragma once

#include <QWidget>
#include "ui_EmotionWindow.h"

class EmotionWindow : public QWidget
{
	Q_OBJECT

public:
	EmotionWindow(QWidget *parent = nullptr);
	~EmotionWindow();
private :
	void initControl();

private slots:
	void addEmotion(int emotionNum);

signals:
	//表情窗口隐藏
	void signalEmotionWindowHide();
	//点击表情窗口
	void signalEmotionItemClicked(int emorionNum);

private:
	void paintEvent(QPaintEvent* event)override; 

private:
	Ui::EmotionWindowClass ui;
};

