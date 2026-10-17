#include "videorenderwidget.h"
VideoRenderWidget::VideoRenderWidget(QWidget *parent)
    : QWidget(parent)
{
    // 开启双缓冲，消除闪烁
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    //m_frame = QImage(1280,720,QImage::Format_RGBA8888);
    //m_frame.fill(Qt::red);
    update();
}

void VideoRenderWidget::onRecvFrame(const QImage &frame)
{
    qDebug() << "收到视频帧尺寸：" << frame.size();
    m_frame = frame;
    // 触发重绘
    update();
}

void VideoRenderWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_frame.isNull())
    {
        // 保持比例缩放，居中渲染
        QRect targetRect = m_frame.rect();
        targetRect.moveCenter(this->rect().center());
        targetRect = targetRect.intersected(this->rect());

        painter.drawImage(this->rect(), m_frame, m_frame.rect());
    }
    else
    {
        // 无画面填充黑色
        painter.fillRect(this->rect(), Qt::black);
    }
}
