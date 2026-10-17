
# 基于Qt的c/s架构的即时通讯系统-客户端

<img width="543" height="397" alt="image" src="https://github.com/user-attachments/assets/681a969c-e5f9-405d-9658-063a629ec659" />


**项目简介： 独立开发的桌面端即时通讯应用，完整实现登录注册、好友管理、添加好友、单聊 / 群聊、发送不同消息类型、实时音视频通话、封装JSON工具函数来实现 聊天消息、缓存图像、好友信息...持久化、 离线消息存储 、图像二次落盘 、已读消息回执 、未读消息红点绘制 等核心功能。**

## ✨客户端功能模块点

**封装无边框窗口基类 BasicWindow 实现业务基类窗口进行复用::** <mark> 内部实现了 自定义标题栏、拖拽、双击最大化 、 QSS 动态换肤模块、QPainter 实现圆形头像裁剪、设置样式表、 重写绘制事件当样式更改通知所有子控件重新绘制等基类方法</mark> ;

**使用MVC 模型视图分离架构** :: <mark>基于QAbstractListModel+ 自定义 ItemDelegate 自绘聊天气泡；QMap 管理多会话消息，支持多类型消息与本地持久化；图片哈希缓存优化，解决 ListView 滚动卡顿，动态计算条目高度实现收发消息左右气泡布局</mark>。

**封装JSON类工具函数::** <mark>通过序列化和反序列化来进去读取和写入实现本地持久化缓存。</mark>

**联系人树形分组::**  <mark>QTreeWidget + 自定义分组节点，双层 QMap 隔离 UI 指针与业务数据；实现右键分组管理；JSON 完成分组持久化；本地缓存优先渲染 </mark>

**实现实时P2P 音视频通话::** <mark>信令媒体分离，TCP 传呼叫信令，UDP 传输 H.264 音视频；Qt 采集摄像头帧，FFmpeg 软编解码，零延迟编码参数；实现 UDP NALU 分片重组，周期下发 SPS/PPS 抗丢包；自定义视频渲染控件；统一封装资源释放逻辑，防止内存泄漏。集成 FFmpeg 实现 P2P 视频通话，H.264 编解码，自定义 UDP 帧协议支持分片重组</mark>

**离线消息服务端缓存和已读回执发送::** <mark>用户登录主动向服务端拉取离线消息,并进行消息未读红点标记(ContactItem 重写 paintEvent 自绘红色角标)，当用户点开会话则向服务端发送已读回执，并删除该未读红点标记.</mark>

**全局文件与 JSON 序列化操作类::** <mark> 统一管理本地缓存目录（头像 / 群成员 / 聊天图片 / 联系人分组 / 服务器配置），自动创建目录；JSON 实现群成员、聊天记录、联系人分组持久化，提供结构体↔JSON、ChatMsg↔QByteArray/QVariantMap 转换工具；图片缩放 JPEG 压缩落盘；时间戳 + 自增 ID 生成唯一文件名。</mark>

**实现离线消息机制::** <mark>用户离线时消息入库，登录后主动请求拉取；图片消息落盘服务端磁盘，以 uuid 作为唯一标识，数据库仅存元信息，客户端二次拉取二进制</mark>

**实现已读回执::** <mark> 客户端打开会话时按会话颗粒度发送回执，服务端标记已读取</mark>

**客户端服务器配置与连接管理面板** <mark> 单例配置面板，JSON 持久化服务器 IP / 端口。</mark>

**实现应用层心跳超时检测::** <mark>客户端定时上报心跳，服务端记录最后活跃时间戳，定时遍历超时连接并同步清理三个映射容器，解决 TCP disconnected 信号无法感知异常断网的问题</mark>

## 🖼️ 客户端效果预览

<img width="803" height="572" alt="动画" src="https://github.com/user-attachments/assets/c033877d-1783-4f51-b520-39fbbd62b145" />

<img width="974" height="734" alt="image" src="https://github.com/user-attachments/assets/505b439b-24ce-4acd-bc16-ed38461c0c0b" />

<img width="894" height="626" alt="动画" src="https://github.com/user-attachments/assets/1bf87290-ef38-493c-ae66-3ab75e0a4ea3" />
<img width="353" height="850" alt="image" src="https://github.com/user-attachments/assets/626f5975-71e6-40bf-98c9-4acabf5d290d" />

<img width="500" height="375" alt="image" src="https://github.com/user-attachments/assets/9632de0d-6bdb-420a-bcb3-8e6e88c416b8" />


