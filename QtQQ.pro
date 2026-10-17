TEMPLATE = app
TARGET = QtQQ

QT += core widgets sql network multimedia
QT += multimedia

CONFIG += c++17

# 资源文件（只添加 .qrc 文件，不要添加生成的 .cpp）
RESOURCES += CCMianWidget.qrc
# FFmpeg 配置
INCLUDEPATH += E:/ffmpeg/ffmpeg-8.1.2-full_build-shared/include
LIBS += -LE:/ffmpeg/ffmpeg-8.1.2-full_build-shared/lib
LIBS += -lavcodec -lavformat -lavutil -lswscale


# UI 文件（只添加 .ui 文件，不要添加生成的 .h）
FORMS += \
    CCMianWidget.ui \
    ConnectServer.ui \
    ContactItem.ui \
    EmotionWindow.ui \
    FriendsDialog.ui \
    ReceiveFile.ui \
    SendFile.ui \
    SkinWindow.ui \
    TalkWindow.ui \
    TalkWindowItem.ui \
    TalkWindowShell.ui \
    UserLogin.ui \
    administratorwidget.ui \
    friends.ui \
    registrationuser.ui \
    videowidget.ui

# 头文件（只添加你手写的 .h，不要添加生成的 ui_*.h）
HEADERS += \
    administratorwidget.h \
    basicwindow.h \
    CCMianWidget.h \
    ChatListModel.h \
    ChatMsg.h \
    ChatMsgDelegate.h \
    CommonUtils.h \
    ContactItem.h \
    CustomMenu.h \
    EmotionLabelItem.h \
    EmotionWindow.h \
    GroupMember.h \
    MsgWebView.h \
    NotifyManager.h \
    QCilickLabel.h \
    QMsgTextEdit.h \
    QPeratioFile.h \
    ReceiveFile.h \
    RootContatItem.h \
    SendFile.h \
    SkinWindow.h \
    Systray.h \
    TalkWindow.h \
    TalkWindowItem.h \
    TalkWindowShell.h \
    TcpClient.h \
    TitleBar.h \
    UserLogin.h \
    WindowManager.h \
    cliickmenu.h \
    friends.h \
    registrationuser.h \
    videocalludp.h \
    videocommdef.h \
    videorenderwidget.h \
    videowidget.h

# 源文件（只添加你手写的 .cpp，不要添加 qrc_*.cpp）
SOURCES += \
    administratorwidget.cpp \
    basicwindow.cpp \
    CCMianWidget.cpp \
    ChatListModel.cpp \
    ChatMsgDelegate.cpp \
    CommonUtils.cpp \
    ContactItem.cpp \
    CustomMenu.cpp \
    EmotionLabelItem.cpp \
    EmotionWindow.cpp \
    cliickmenu.cpp \
    friends.cpp \
    main.cpp \
    MsgWebView.cpp \
    NotifyManager.cpp \
    QCilickLabel.cpp \
    QMsgTextEdit.cpp \
    QPeratioFile.cpp \
    ReceiveFile.cpp \
    RootContatItem.cpp \
    SendFile.cpp \
    SkinWindow.cpp \
    Systray.cpp \
    TalkWindow.cpp \
    TalkWindowItem.cpp \
    TalkWindowShell.cpp \
    TcpClient.cpp \
    TitleBar.cpp \
    UserLogin.cpp \
    WindowManager.cpp \
    registrationuser.cpp \
    videocalludp.cpp \
    videorenderwidget.cpp \
    videowidget.cpp
