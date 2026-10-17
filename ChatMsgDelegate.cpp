#include "ChatMsgDelegate.h"
# include "ChatListModel.h"
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include <QVariant>
#include <QImage>
# include <QPainterPath>

// 静态缓存全局初始化，程序启动时执行一次
QMap<QString, QPixmap> ChatMsgDelegate::m_picCache;
QMap<QString, QPixmap> ChatMsgDelegate::m_avatarCircleCache;

// 默认头像 懒加载单例
QPixmap& ChatMsgDelegate::defaultAvatar()
{
    static QPixmap ins;
    return ins;
}

/**
 * @brief 头像裁剪路径静态初始化：生成固定45*45正圆路径，全程只构造一次
 */
const QPainterPath ChatMsgDelegate::s_avatarClipPath = []()
 {
        QPainterPath p;
        // 从左上角(0,0)绘制宽高等于头像尺寸的内切椭圆，生成正圆裁剪区域
        p.addEllipse(0, 0, 45, 45);
        return p;
    }();

ChatMsgDelegate::ChatMsgDelegate(QObject* parent)
        : QStyledItemDelegate(parent)
{
    }

    /**
     * @brief 从缓存获取指定路径+尺寸的图片，不存在则加载+缩放+存入缓存
     */
QPixmap ChatMsgDelegate::getCachedPix(const QString& path, int w, int h) const
    {
        // 拼接唯一键：路径+宽度+高度，同一张图不同缩放尺寸分开缓存
        const QString key = QString("%1|%2|%3").arg(path).arg(w).arg(h);

        // 如果哈希表中已经存在该图片，直接查表返回，无需磁盘读取
        if (m_picCache.contains(key))
        {
            return m_picCache[key];
        }

        // 首次需要加载：从本地文件读取图片
        QPixmap pix(path);

        // 文件损坏/路径不存在，填充灰色占位图防止空白绘制
        if (pix.isNull())
        {
            //当路径不存在时，先不存入静态哈希缓存。等待服务器下发离线图像数据
            pix.fill(Qt::gray);
            QPixmap scaled = pix.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            return scaled;

        }else{
        // 等比例平滑缩放到目标尺寸
        QPixmap scaled = pix.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // 存入静态缓存，后续滚动直接取用
        m_picCache.insert(key, scaled);
        return scaled;
        }
}

    /**
     * @brief 获取预裁剪完成的圆形头像，优先读缓存，无缓存则生成并存入
     */
QPixmap ChatMsgDelegate::getCachedAvatar(const QString& avatarBin) const
{
        // 二进制数据为空，使用默认纯色头像
        if (avatarBin.isEmpty())
        {
            auto&  m_defaultAvatar = defaultAvatar();
            // 默认头像只初始化一次，后续永久复用
            if (m_defaultAvatar.isNull())
            {
                // 基础纯色方块
                QPixmap base(AvatarSize, AvatarSize);
                base.load(":/Resources/MainWindow/girl.png");

                // 新建透明画布存放圆形头像
                QPixmap circle(AvatarSize, AvatarSize);
                circle.fill(Qt::transparent);
                QPainter p(&circle);
                p.setRenderHint(QPainter::Antialiasing);
                p.save();
                // 绑定全局静态裁剪圆路径
                p.setClipPath(s_avatarClipPath);
                // 铺满画布缩放，无留白
                p.drawPixmap(0, 0, base.scaled(AvatarSize, AvatarSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                p.restore();

                m_defaultAvatar = circle;
            }
            return m_defaultAvatar;
        }

        // 该二进制头像已经缓存过，直接取出
        if (m_avatarCircleCache.contains(avatarBin))
        {
            return m_avatarCircleCache[avatarBin];
        }

        // 第一次解析二进制图片
        QPixmap src;
        src.load(avatarBin);
        // 二进制解析失败兜底填充纯色
        if (src.isNull())
        {
            src.fill(":/Resources/MainWindow/girl.png");
        }

        // 生成透明画布，裁剪圆形
        QPixmap circleAvatar(AvatarSize, AvatarSize);
        circleAvatar.fill(Qt::transparent);
        QPainter p(&circleAvatar);
        p.setRenderHint(QPainter::Antialiasing);
        p.save();
        p.setClipPath(s_avatarClipPath);
        p.drawPixmap(0, 0, src.scaled(AvatarSize, AvatarSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.restore();

        // 存入哈希缓存，下次滚动不再重复裁剪计算
        m_avatarCircleCache.insert(avatarBin, circleAvatar);
        return circleAvatar;
    }

    /**
     * @brief 计算单条消息条目整体高度，ListView依靠此函数排布每行高度
     * @note 滚动过程会频繁调用该函数，目前未做尺寸缓存，极致优化可把计算结果写入ChatMsg结构体
     */
QSize ChatMsgDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
        // 从Model索引取出绑定的消息结构体
        QVariant var = index.data(ChatListModel::MsgRole);
        ChatMsg msg = var.value<ChatMsg>();

        // 字体测量工具，用来计算文字换行占用区域
        QFontMetrics fm(option.font);
        // 气泡最大可用宽度：总条目宽度减去左右头像占位与四边外边距
        int maxBubbleWidth = option.rect.width() - AvatarSize * 2 - Margin * 4;

        int contentH = 0;
        // 根据消息类型分别计算气泡内部内容高度
        if (msg.msgType == ChatMsg::TextMsg)
        {
            // 在限定宽度内自动换行，计算整块文字矩形大小
            QRect textRect = fm.boundingRect(0, 0, maxBubbleWidth, 9999, Qt::TextWordWrap, msg.content);
            // 高度 = 昵称行 + 正文文字 + 时间行 + 上下内边距
            contentH = fm.height() + textRect.height() + fm.height() + BubblePadding * 2;
        }
        else if (msg.msgType == ChatMsg::ImageMsg)
        {
            // 图片固定最大高度+昵称+内边距
            contentH = fm.height() + MaxImageHeight + BubblePadding * 2;
        }
        else if (msg.msgType == ChatMsg::EmojiMsg)
        {
            // 表情固定尺寸+昵称+内边距
            contentH = fm.height() + EmojiSize + BubblePadding * 2;
        }

        // 条目整体高度取头像高度与内容高度的较大值，保证不会头像超出条目范围
        int itemH = qMax(AvatarSize + Margin * 2, contentH + Margin * 2);
        // 宽度跟随列表整行宽度，高度为计算出的自适应高度
        return QSize(option.rect.width(), itemH);
    }

    /**
     * @brief 顶层绘制总入口，统一初始化画笔基础配置，区分我方/对方消息分支
     */
void ChatMsgDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        // 全局开启抗锯齿，所有圆弧、图片缩放全部生效，无需每个子绘制重复开启
        painter->setRenderHint(QPainter::Antialiasing);
        // 填充条目背景纯白色，清除ListView默认选中高亮底色
        painter->fillRect(option.rect, Qt::white);

        // 读取Model绑定的消息数据
        QVariant var = index.data(ChatListModel::MsgRole);
        ChatMsg msg = var.value<ChatMsg>();

        // isSelf=true：自己发的消息靠右绘制；false：对方消息靠左绘制
        if (msg.isSelf)
            drawRightMsg(painter, option, msg);
        else
            drawLeftMsg(painter, option, msg);
    }

    /**
     * @brief 绘制【我方发送消息】：头像在最右侧、气泡居左浅蓝色
     */
void ChatMsgDelegate::drawRightMsg(QPainter* painter, const QStyleOptionViewItem& option, const ChatMsg& msg) const
{
    // 获取当前画笔字体，用于文字绘制与尺寸测算
    QFont font = painter->font();
    QFontMetrics fm(font);
    // ===== 局部临时变量，当场计算，不用头文件定义 =====
    int maxBubbleW = option.rect.width() - AvatarSize * 2 - Margin * 4;

    // ===================== 1 绘制右侧圆形头像 =====================
    // 计算头像坐标：条目最右侧向左偏移头像宽+外边距，顶部向下偏移外边距
    QRect avatarRect(option.rect.right() - AvatarSize - Margin, option.rect.top() + Margin, AvatarSize, AvatarSize);
    // 直接从缓存取出已经裁剪完成的圆形头像，无临时画布、无重复裁剪
    QPixmap roundAvatar = getCachedAvatar(msg.avatarData);
    // 把头像绘制到对应矩形位置
    painter->drawPixmap(avatarRect, roundAvatar);

    // ===================== 2 提前计算气泡最终宽高+坐标【最关键前置】 =====================
    int bubbleW = 0;
    int bubbleH = 0;
    QRect textBounds;

    if (msg.msgType == ChatMsg::TextMsg)
    {
        // 测算文字换行后占用区域
        textBounds = fm.boundingRect(0, 0, maxBubbleW - BubblePadding * 2, 9999, Qt::TextWordWrap, msg.content);
        bubbleW = textBounds.width() + BubblePadding * 2;
        bubbleH = fm.height()      // 昵称行
            + textBounds.height() // 正文行
            + fm.height()      // 时间行
            + BubblePadding * 2; // 上下整体内边距
    }
    else if (msg.msgType == ChatMsg::ImageMsg)
    {
        bubbleW = MaxImageWidth + BubblePadding * 2;
        bubbleH = fm.height() + MaxImageHeight + BubblePadding * 2;
    }
    else if (msg.msgType == ChatMsg::EmojiMsg)
    {
        bubbleW = EmojiSize + BubblePadding * 2;
        bubbleH = fm.height() + EmojiSize + BubblePadding * 2;
    }

    // 定位气泡：气泡右侧贴着头像，只留Margin缝隙，向左拓展气泡宽度
    int bubbleRightX = avatarRect.left() - Margin;
    int bubbleLeftX = bubbleRightX - bubbleW;
    // 一次性生成最终固定的气泡矩形，后续绝不修改
    QRect bubbleRect(bubbleLeftX, option.rect.top() + Margin, bubbleW, bubbleH);

    // ===================== 【第一步先画气泡底色！底色垫底，文字后画】 =====================
    painter->setPen(Qt::NoPen);        // 取消边框黑线
    painter->setBrush(QColor(130, 190, 255)); // 我方气泡浅蓝色
    painter->drawRoundedRect(bubbleRect, 8, 8); // 先把背景画出来

    // 内容绘制起始Y坐标：气泡顶部 + 上内边距
    int drawTop = bubbleRect.top() + BubblePadding;

    // ===================== 3 绘制气泡顶部发送者昵称 =====================
    painter->setPen(Qt::black);
    // 左右预留内边距，宽度减掉两侧Padding，防止文字超出气泡
    QRect nameRect(bubbleRect.left() + BubblePadding, drawTop, bubbleW - BubblePadding * 2, fm.height());
    // 单行昵称超长自动末尾省略 ...
    painter->drawText(nameRect, Qt::TextSingleLine | Qt::ElideRight, msg.userName);

    // Y轴下移：昵称高度 + 分隔内边距
    drawTop += fm.height() + BubblePadding;

    // ===================== 4 按消息类型绘制气泡内部主体内容 =====================
    if (msg.msgType == ChatMsg::TextMsg)
    {
        // 正文绘制区域，左右扣掉内边距
        QRect contentRect(
            bubbleRect.left() + BubblePadding,
            drawTop,
            bubbleRect.width() - BubblePadding * 2,
            textBounds.height()
        );
        painter->drawText(contentRect, Qt::TextWordWrap, msg.content);
    }
    else if (msg.msgType == ChatMsg::ImageMsg)
    {
        QPixmap img = getCachedPix(msg.resPath, MaxImageWidth, MaxImageHeight);
        painter->drawPixmap(bubbleRect.left() + BubblePadding, drawTop, img);
    }
    else if (msg.msgType == ChatMsg::EmojiMsg)
    {
        QPixmap emoji = getCachedPix(msg.resPath, EmojiSize, EmojiSize);
        painter->drawPixmap(bubbleRect.left() + BubblePadding, drawTop, emoji);
    }

    // ===================== 5 在气泡最底部绘制发送时间 =====================
    painter->setPen(Qt::darkGray);
    QRect timeRect(
        bubbleRect.left() + BubblePadding,
        bubbleRect.bottom() - fm.height() - BubblePadding,
        bubbleRect.width() - BubblePadding * 2,
        fm.height()
    );
    painter->drawText(timeRect, Qt::TextSingleLine | Qt::ElideRight, msg.timeStr);

}

// 静态接口，外部随时调用清空全部图片缓存
void ChatMsgDelegate::clearAllCache()
    {
        // 1. 清空聊天图片、表情缓存
        m_picCache.clear();
        // 2. 清空所有用户头像圆形缓存
        m_avatarCircleCache.clear();
        // 3. 重置默认头像，释放图片内存
        defaultAvatar() = QPixmap();
    }

    /**
     * @brief 绘制【对方发来消息】：头像在最左侧、气泡居右灰色底色*/
 void ChatMsgDelegate::drawLeftMsg(QPainter *painter, const QStyleOptionViewItem &option, const ChatMsg &msg) const
{
    // 获取当前绘图字体，创建字体测量工具
    QFont font = painter->font();
    QFontMetrics fm(font);

    // 计算气泡单行最大允许宽度（整行宽度扣除两边头像与外边距）
    int maxBubbleW = option.rect.width() - AvatarSize * 2 - Margin * 4;

    // ===================== 1 绘制左侧对方头像 =====================
    QRect avatarRect(option.rect.left() + Margin, option.rect.top() + Margin, AvatarSize, AvatarSize);
    // 读取缓存好的圆形头像，无需重复裁剪绘制
    QPixmap roundAvatar = getCachedAvatar(msg.avatarData);
    painter->drawPixmap(avatarRect, roundAvatar);

    // ===================== 2 【前置先计算气泡最终宽高，不提前占位】 =====================
    int bubbleW = 0;
    int bubbleH = 0;
    QRect textBounds;

    if (msg.msgType == ChatMsg::TextMsg)
    {
        // 测算文字自动换行后的整体占用尺寸
        textBounds = fm.boundingRect(0, 0, maxBubbleW - BubblePadding * 2, 9999, Qt::TextWordWrap, msg.content);
        bubbleW = textBounds.width() + BubblePadding * 2;
        bubbleH = fm.height() + textBounds.height() + fm.height() + BubblePadding * 2;
    }
    else if (msg.msgType == ChatMsg::ImageMsg)
    {
        bubbleW = MaxImageWidth + BubblePadding * 2;
        bubbleH = fm.height() + MaxImageHeight + BubblePadding * 2;
    }
    else if (msg.msgType == ChatMsg::EmojiMsg)
    {
        bubbleW = EmojiSize + BubblePadding * 2;
        bubbleH = fm.height() + EmojiSize + BubblePadding * 2;
    }

    // 气泡紧贴头像右侧，中间只留Margin间隙，向右拓展气泡宽度
    QRect bubbleRect(avatarRect.right() + Margin, option.rect.top() + Margin, bubbleW, bubbleH);

    // ===================== 核心：先画气泡背景底色！底层先铺颜色，文字后画不会被覆盖 =====================
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(230, 230, 230));
    painter->drawRoundedRect(bubbleRect, 8, 8);

    // 内容绘制起始纵向Y坐标
    int drawTop = bubbleRect.top() + BubblePadding;

    // ===================== 3 绘制发送者昵称，宽度限定为气泡实际宽度，不会越界 =====================
    painter->setPen(Qt::black);
    // 左右减去内边距，防止文字贴边框，超长昵称自动末尾省略
    QRect nameDrawRect(
        bubbleRect.left() + BubblePadding,
        drawTop,
        bubbleW - BubblePadding * 2,
        fm.height()
    );
    painter->drawText(nameDrawRect, Qt::TextSingleLine | Qt::ElideRight, msg.userName);

    // 昵称下方留出间距，准备绘制正文内容
    drawTop += fm.height() + BubblePadding;

    // ===================== 4 分消息类型绘制正文/图片/表情，不再二次修改bubbleRect =====================
    if (msg.msgType == ChatMsg::TextMsg)
    {
        // 直接使用之前已经计算好的文字区域，不再重复计算与修改气泡大小
        QRect contentRect(
            bubbleRect.left() + BubblePadding,
            drawTop,
            bubbleRect.width() - BubblePadding * 2,
            textBounds.height()
        );
        painter->drawText(contentRect, Qt::TextWordWrap, msg.content);
    }
    else if (msg.msgType == ChatMsg::ImageMsg)
    {
        QPixmap img = getCachedPix(msg.resPath, MaxImageWidth, MaxImageHeight);
        painter->drawPixmap(bubbleRect.left() + BubblePadding, drawTop, img);
    }
    else if (msg.msgType == ChatMsg::EmojiMsg)
    {
        QPixmap emoji = getCachedPix(msg.resPath, EmojiSize, EmojiSize);
        painter->drawPixmap(bubbleRect.left() + BubblePadding, drawTop, emoji);
    }

    // ===================== 5 绘制底部时间文字 =====================
    painter->setPen(Qt::gray);
    QRect timeDrawRect(
        bubbleRect.left() + BubblePadding,
        bubbleRect.bottom() - fm.height() - BubblePadding,
        bubbleRect.width() - BubblePadding * 2,
        fm.height()
    );
    painter->drawText(timeDrawRect, Qt::TextSingleLine | Qt::ElideRight, msg.timeStr);
}
