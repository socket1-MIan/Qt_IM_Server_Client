#ifndef VIDEORENDERWIDGET_H
#define VIDEORENDERWIDGET_H
#include <QWidget>
#include <QImage>
#include <QPainter>

class VideoRenderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoRenderWidget(QWidget *parent = nullptr);

public slots:
    // 接收远端视频图像
    void onRecvFrame(const QImage& frame);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_frame;
};
#endif
