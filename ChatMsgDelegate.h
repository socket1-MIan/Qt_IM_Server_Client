#pragma once

#include <QWidget>

#include <QStyledItemDelegate>
#include "ChatMsg.h"

struct ChatMsg;

class ChatMsgDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatMsgDelegate(QObject* parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    // 静态接口，外部随时调用清空全部图片缓存
    static void clearAllCache();
private:
    void drawLeftMsg(QPainter* painter, const QStyleOptionViewItem& option, const ChatMsg& msg) const;
    void drawRightMsg(QPainter* painter, const QStyleOptionViewItem& option, const ChatMsg& msg) const;

    // 从缓存获取缩放好的图片
    QPixmap getCachedPix(const QString& path, int w, int h) const;
    // 根据头像二进制获取已经裁剪好的圆形头像
    QPixmap getCachedAvatar(const QString& avatarBin) const;

    // 常量定义
    const int AvatarSize = 42;
    const int Margin = 3;    // 头像与气泡中间间隔3像素
    const int BubblePadding = 5;
    const int MaxImageWidth = 220;
    const int MaxImageHeight = 300;
    const int EmojiSize = 36;

    // 静态全局缓存：程序运行期间只加载一次
    static QMap<QString, QPixmap> m_picCache;
    static QMap<QString, QPixmap> m_avatarCircleCache;
    // 默认纯色头像只生成一次
    static QPixmap& defaultAvatar();
    // 头像裁剪路径只构造一次
    static const QPainterPath s_avatarClipPath;
};
