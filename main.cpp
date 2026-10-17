#include "CCMianWidget.h"
#include "UserLogin.h"
#include "WindowManager.h"
# include "TalkWindow.h"

#include <QtWidgets/QApplication>
#include <windows.h>

int main(int argc, char *argv[])
 {

     // 在QApplication创建之前设置DLL搜索路径！
     QString ffmpegDir = QCoreApplication::applicationDirPath() + "/ffmpeg";
     SetDllDirectoryW(ffmpegDir.toStdWString().c_str());

    QApplication app(argc, argv);
    //CCMianWidget window;
    //window.show();
    //当最后一个窗口关闭得时候在退出
    app.setQuitOnLastWindowClosed(false);

    // 先拿到两个单例，保证构造完成
    auto tcp = TcpClient::getInstance();
    auto admin = AdministratorWidget::getInstance();
   // auto winMen = WindowManager::getInstance();
    ChatListModel::getInstance(qApp);  // 以app为parent，程序结束才释放

    //确保2个单列对象，都提前创建防止相互调用导致递归
    tcp->initSignalBind();

    // 在 main 函数中或某个初始化函数中
    //在 Qt 的元类型系统中注册 QList<QList<QString>> 类型，
    // 以便该类型能够被 QVariant 正确存储、序列化和反序列化
    // （例如用于 QSettings、QDataStream 或跨线程信号槽）。
    qRegisterMetaType<QList<QStringList>>("QList<QStringList>");
    qRegisterMetaType<QMap<QString, QByteArray>>("QMap<QString, QByteArray>");
    qRegisterMetaType<QList<QVariantMap>>("QList<QVariantMap>");
    qRegisterMetaType<QList<QMap<QString, QVariant>>>("QList<QMap<QString,QVariant>>");


    UserLogin* usserLogin = new UserLogin;
    usserLogin->show();
    return app.exec();
}
