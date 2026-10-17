#include "ChatListModel.h"
#include "QPeratioFile.h"
#include "WindowManager.h"
ChatListModel::ChatListModel(QObject* parent)
    : QAbstractListModel(parent)
{

}
// 返回当前模型的行数
int ChatListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_curMsgList)
        return 0;
    return m_curMsgList->size();
}


QVariant ChatListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_curMsgList || index.row() >= m_curMsgList->size())
        return QVariant();

    const ChatMsg& msg = m_curMsgList->at(index.row());
    if (role == MsgRole)
    {
        return QVariant::fromValue(msg);
    }
    return QVariant();
}

// 切换联系人，重置视图数据
void ChatListModel::switchContact(const QString& contactId)
{
    beginResetModel(); // 通知视图整体重置数据
    m_curContactId = contactId;
    // 不存在该联系人则自动创建空消息列表
    if (!m_contactMsgMap.contains(contactId))
    {
        //没有创建的话，先从缓存数据中拿取
        m_contactMsgMap.insert(contactId, 
            QPeratioFile::instance()->TakeMessageFile(contactId));
    }
    m_curMsgList = &m_contactMsgMap[contactId];
    endResetModel(); // 视图自动刷新界面
}

//将聊天记录写入本地缓存
void ChatListModel::SwitchFile()
{
    QMap<QString, QList<ChatMsg>>::const_iterator i_r;
    i_r = m_contactMsgMap.constBegin();
    for (; i_r != m_contactMsgMap.constEnd(); i_r++) {
      bool isEnpy = QPeratioFile::instance()->SwitchFile(i_r.key(), i_r.value());
      if (!isEnpy) {
          qDebug() << "当前写入聊天记录数据 %d失败" << i_r.key();
      }

    }

}

//将聊天消息写入到缓存文件中
void ChatListModel::SwitchFile(const QString& uid)
{
    if (m_contactMsgMap.contains(uid)) {
        QList<ChatMsg> member = m_contactMsgMap.value(uid);
        bool isEnpy = QPeratioFile::instance()->SwitchFile(uid, member);
        if (!isEnpy) {
            qDebug() << "当前写入聊天记录数据 %d失败" << uid;
        }

    }
}

//只能在外部追加离线消息的时候调用
void ChatListModel::addRecvMsg(const ChatMsg &Message)
{

    ChatMsg msg =  Message;
    QString Name = msg.send;	//发送这条消息的对象

    //自动存入对应联系人缓存,如果是当前聊天的活动窗口则立刻刷新UI
    appendContactMsg(Name, msg);
}


// 给指定联系人新增消息（收发消息统一调用）
void ChatListModel::appendContactMsg(const QString& contactId, const ChatMsg& msg)
{
    // 消息归属当前正在查看的联系人，实时刷新UI
    if (contactId == m_curContactId && m_curMsgList)
    {
        beginInsertRows(QModelIndex(), m_curMsgList->size(), m_curMsgList->size());
        m_curMsgList->append(msg);
        endInsertRows();
        return;
    }

    // 不是当前联系人，仅后台缓存，不刷新界面
    // 修复：如果该联系人还没加载过历史记录，先从本地文件加载
    if (!m_contactMsgMap.contains(contactId))
    {
        m_contactMsgMap.insert(contactId,
                               QPeratioFile::instance()->TakeMessageFile(contactId));
    }
    m_contactMsgMap[contactId].append(msg);
    //添加未读消息+1
    WindowManager::getInstance()->updateSessionBadge(contactId, 1);
}



void ChatListModel::clearCurrentMsg()
{
    if (!m_curMsgList) return;
    beginResetModel();

    m_curMsgList->clear();
    //SwitchFile();
    endResetModel();
}
