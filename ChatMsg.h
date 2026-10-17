#pragma once

#include <QString>
#include <QByteArray>
# include <QPixmap>

// 单条消息数据结构体
struct ChatMsg
{

    QString userName;           // 发送者昵称
    QString timeStr;           // 发送时间
    QString avatarData;       // 头像路径
    //QString strImageURL;   //头像的缓存数据
    bool isSelf = false;    // true=我方消息（靠右） false=对方消息（靠左）
    QString receive;       //接收对象
    QString send;          //发送对象
    enum  MsgType
    {
        TextMsg = 0,        //普通文本信息
        ImageMsg = 1,      //图片信息
        EmojiMsg = 2      //表情信息

    };
    int msgType = TextMsg;      // 消息类型 0文本 1图片 2表情 后续可扩展
    QString resPath;           // 图片/表情本地缓存文件路径
    //QByteArray ImageData;     // 图片 表情数据
    QString content;          // 消息文本内容
};
