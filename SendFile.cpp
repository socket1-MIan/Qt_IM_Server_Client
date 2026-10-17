#include "SendFile.h"
# include "WindowManager.h"
# include "TalkWindowShell.h"

# include <QFileDialog>
# include <QMessageBox>

SendFile::SendFile(QWidget* parent)
	: BasicWindow(parent)
	,m_filePath("")
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);
	initTitleBar();
	setTitleBarTitle("", ":/Resources/MainWindow/app/logo.ico");
	loadStyleSheet("SendFile");

	this->move(100, 400);

    //TalkWindowShell* talkWindowShell = WindowManager::getInstance()->getTalkWindowShell();

	
}

SendFile::~SendFile()
{}


//发送文件
void SendFile::on_sendBtn_clicked()
{
	
	if (!m_filePath.isEmpty()) {
		QFile file(m_filePath);
		if (file.open(QFile::ReadOnly)) {

			int msgType = 2;
			QString str = file.readAll();

			//获取文件的名称
			QFileInfo fileInfo(m_filePath);
			QString fileName = fileInfo.fileName();

			emit sendFileClicked(str, msgType, fileName);

			file.close();
		}
		else
		{
			QMessageBox::information(this,
				QString::fromUtf8("提示"),
				QString::fromUtf8("读取文件:%1失败").arg(m_filePath));
			m_filePath = " ";
			ui.lineEdit->clear();

			this->close();
			return;

		}

		m_filePath = " ";
		this->close();
	}

}
//打开文件
void SendFile::on_openBtn_clicked() {

	m_filePath = QFileDialog::getOpenFileName(
		this,
		QString::fromUtf8("选择文件"),
		"/",
		QString::fromUtf8("发送的文件(*.txt *.doc);;所有文件(*.*);;")
	);
	ui.lineEdit->setText(m_filePath);
}
