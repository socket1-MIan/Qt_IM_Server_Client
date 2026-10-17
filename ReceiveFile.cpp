#include "ReceiveFile.h"

# include <QFileDialog>
# include <QMessageBox>

extern QString gfileName;
extern QString gfileData; 

ReceiveFile::ReceiveFile(QWidget *parent)
	: BasicWindow(parent)
{
	ui.setupUi(this);

	this->setAttribute(Qt::WA_DeleteOnClose);
	initTitleBar();
	setTitleBarTitle("", ":/Resources/MainWindow/app/logo.ico");
	loadStyleSheet("ReceiveFile");

	this->move(100, 400);

	//用户点击标题窗口的X 关闭窗口并拒绝接收文件信息 
	connect(_titleBar, &TitleBar::signalButtonCloseClicked, 
		this, &ReceiveFile::refuseFile);
}

ReceiveFile::~ReceiveFile()
{}

//设置的信息
void ReceiveFile::setMsg(QString & msgLabel)
{
	ui.label->setText(msgLabel);
}

//确认接收文件
void ReceiveFile::on_okBtn_clicked()
{
	this->close(); 

	//获取要保存的文件路径
	QString fileDirPath = QFileDialog::getExistingDirectory(
		nullptr,
		QStringLiteral("文件保存路径"),
		"/"
	);

	QString filePath = fileDirPath + "/" + gfileName; 

	QFile file(filePath);
	if (file.open(QFile::WriteOnly)) {
		
		file.write(gfileData.toUtf8());
		file.close(); 
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("文件接收成功")
		);
	}
	else {
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("文件接收失败")
		);
	}
	

}

//取消接收文件
void ReceiveFile::on_canelBtn_clicked() {

	emit refuseFile(); //发送拒绝接收文件的信号
	this->close(); 
}