#include "SkinWindow.h"
//皮肤窗口
SkinWindow::SkinWindow(QWidget *parent)
	: BasicWindow(parent)
{
	ui.setupUi(this); 
	loadStyleSheet("SkinWindow");
	setAttribute(Qt::WA_DeleteOnClose);
	initControl();

}

SkinWindow::~SkinWindow()
{}

void SkinWindow::initControl()
{

	QList<QColor> colorList = {
		QColor(22,154,218), QColor(40,138,221), QColor(49,166,107),QColor(218,67,68),
		QColor(177,99,158), QColor(107,81,92),	QColor(89,92,160), QColor(21,156,199),
		QColor(79,169,172), QColor(155,183,154),QColor(128,77,77), QColor(240,188,189)
	};

	for (int row = 0; row < 3; row ++) {
		for (int column = 0; column < 4; column++) {

			QCilickLabel* label = new QCilickLabel(this);
			connect(label, &QCilickLabel::clicked, this, [=]() {
				NotifyManager::getInstance()->notifyOtherWindowChangeSkin(colorList.at(row * 4 + column));
				
				});
			label->setFixedSize(84, 84);

			QPalette plaette;
			plaette.setColor(QPalette::Window, colorList.at(row * 4 + column));
			label->setAutoFillBackground(true);
			label->setPalette(plaette);

			ui.gridLayout->addWidget(label, row, column);

		}
	}

	connect(ui.symin, &QPushButton::clicked, this, &SkinWindow::onShowMin);
	connect(ui.syslose, &QPushButton::clicked, this, &SkinWindow::onShowClose);

}

void SkinWindow::onShowClose()
{
	close(); 
}
