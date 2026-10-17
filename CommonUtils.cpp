#include "CommonUtils.h"
# include <QFile>
# include <QApplication>
# include <QSettings>

CommonUtils::CommonUtils()
{

}
// 圆头像 
QPixmap CommonUtils::getRoundIamge(const QPixmap& src, QPixmap& mask, QSize masksize)
{
	if (masksize == QSize(0, 0))
		masksize = mask.size();
	else
	{
		mask = mask.scaled(masksize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		
	}
	
	QImage resultIamage(masksize, QImage::Format_ARGB32_Premultiplied);
	QPainter painter(&resultIamage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultIamage.rect(), Qt::transparent);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawPixmap(0, 0, mask);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawPixmap(0, 0, src.scaled(masksize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	painter.end();
	return QPixmap::fromImage(resultIamage);

}
//加载样式表 
void CommonUtils::loadStyleSheet(QWidget* widget, const QString& sheetName)
{
	QFile file(":/Resources/QSS/" + sheetName + ".css");
	file.open(QFile::ReadOnly);
	if (file.isOpen()) {

		widget->setStyleSheet(" ");
		QString qsstyleSheet = QLatin1String(file.readAll());
		widget->setStyleSheet(qsstyleSheet);
	}
	else {
		qDebug() << "在CommonUtis 45行 QFile.open()打开失败";
	}
	file.close(); 
}

// 设置皮肤的默认颜色
void CommonUtils::setDefaultSkinColor(const QColor& color)
{
	//QApplication::applicationDirPath()返回应用程序可执行文件所在的目录路径
	const QString&& path = QApplication::applicationDirPath() + "/" + QString("tradeprintinfo.ini");
	QSettings settings(path, QSettings::IniFormat);
	
	settings.setValue("Defaultskin/red", color.red());
	settings.setValue("Defaultskin/green", color.green());
	settings.setValue("Defaultskin/blue", color.blue());
	
}

// 获取颜色
QColor CommonUtils::getDefaultSkinColor()
{
	QColor color;
	const QString&& path = QApplication::applicationDirPath() + "/" +
		QString("tradeprintinfo.ini");

	if (!QFile::exists(path)) 
		setDefaultSkinColor(QColor(22, 154, 218));
	
	QSettings settings(path, QSettings::IniFormat);

	color.setRed(settings.value("DefaultSkin/red").toInt());
	color.setGreen(settings.value("DefaultSkin/green").toInt());
	color.setBlue(settings.value("DefaultSkin/blue").toInt());
	
	settings.sync(); // 写入文件
	return color;
}

	