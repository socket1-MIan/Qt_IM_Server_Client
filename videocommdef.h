#ifndef VIDEOCOMMDEF_H
#define VIDEOCOMMDEF_H

#include <QString>
#include <QVariantMap>

// 通话指令码
enum VideoCmd
{
    Cmd_VideoCall_Invite    = 101,  // 发起呼叫
    Cmd_VideoCall_Accept    = 102,  // 接受呼叫
    Cmd_VideoCall_Reject    = 103,  // 拒绝
    Cmd_VideoCall_Close     = 104,  // 挂断
    Cmd_VideoCall_UdpAddr   = 107   // 推送本机UDP监听IP+端口给对方
};

//通话状态击机
enum CallState
{
    Call_Idle,          // 空闲：无通话
    Call_WaitAccept,    // 我发起呼叫，等待对方接听
    Call_InComing,      // 收到别人来电，等待我接听
    Call_Connected      // 通话接通，双向视频中
};

inline QVariantMap createVideoSignal(int cmd, const QString &from, const QString &to, const QString &payload)
{
    QVariantMap map;
    map["cmd"] = cmd;           //指令
    map["fromAccount"] = from;  //发送者的 手机号
    map["toAccount"] = to;      //接收者的 手机号
    map["payload"] = payload;   //推送的的  ip端口号
    return map;
}

#endif // VIDEOCOMMDEF_H
