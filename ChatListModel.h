#pragma once

#include <QWidget>

#include <QAbstractListModel>
#include "ChatMsg.h"
#include <QList>
#include <QMap>

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // 单例接口
    static ChatListModel* getInstance(QObject* parent = nullptr)
    {
        static ChatListModel* instance = nullptr;
        if (!instance)
        {
            instance = new ChatListModel(parent);
        }
        return instance;
    }
    // 禁止拷贝
    ChatListModel(const ChatListModel&) = delete;
    ChatListModel& operator=(const ChatListModel&) = delete;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    enum CustomRole
    {
        MsgRole = Qt::UserRole + 1,
    };
  
    // 给指定联系人追加一条消息
    void appendContactMsg(const QString& contactId, const ChatMsg& msg);

    void clearCurrentMsg(); // 清空当前联系人记录
    void switchContact(const QString& contactId);    // 切换当前查看的联系人，刷新视图
    void SwitchFile();                  //将所有聊天记录写入本地缓存
    void SwitchFile(const QString& uid);//将聊天记录写入本地缓存
    // 新增：获取当前联系人ID（外部可能需要）
    QString getCurrentContactId() const { return m_curContactId; }

    void addRecvMsg(const ChatMsg& Message); //外部调用追加离线消息

private:
     explicit ChatListModel(QObject* parent = nullptr);

private:
    // 所有联系人聊天记录总缓存
    QMap<QString, QList<ChatMsg>> m_contactMsgMap;
    // 当前正在展示的联系人ID
    QString m_curContactId;
    // 当前视图正在渲染的消息列表（指针引用，快捷访问）
    QList<ChatMsg>* m_curMsgList = nullptr;
};
