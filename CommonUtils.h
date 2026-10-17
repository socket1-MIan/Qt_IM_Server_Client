#pragma once

# include <QWidget>
# include <QProxyStyle>
# include <QPainter>

//改变默认的控件的风格
class  CustomProxyStyle : public QProxyStyle
{
	Q_OBJECT
public:
	CustomProxyStyle(QObject* parent) {
		setParent(parent);
	 }
	~ CustomProxyStyle ();

	virtual void drawPrimitice(PrimitiveElement element, const QStyleOption* option,
		QPainter* painter, const QWidget* widget = nullptr)const {

		if (PE_FrameFocusRect == element) {			
			
			// 去掉 windows 中部件的边框虚线框， 部件获取焦点时返回
			// 不进行绘制
			return; 
		}
		else
		{
			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}
	}

};

class CommonUtils
{
public :
	CommonUtils();
public:
	// 圆头像 
	static QPixmap getRoundIamge(const QPixmap& src, QPixmap& mask,
		QSize masksize = QSize(0, 0));

	//加载样式表 
	static void loadStyleSheet(QWidget* widget, const QString& sheetName);

	// 设置皮肤的默认颜色
	static void setDefaultSkinColor(const QColor& color);

	// 获取颜色
	static QColor getDefaultSkinColor(); 


};

