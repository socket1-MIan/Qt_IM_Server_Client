#ifndef ADMINISTRATORWIDGET_H
#define ADMINISTRATORWIDGET_H

#include <QWidget>
# include "BasicWindow.h"
# include "QPeratioFile.h"
# include "TcpClient.h"

//服务器配置结构体
struct ServerConfig
{
    QString ip;
    quint16 port;
    //默认参数
    ServerConfig()
    {
        ip = "127.0.0.1";
        port = 8899;
    }
};

namespace Ui {
class AdministratorWidget;
}

class AdministratorWidget : public BasicWindow
{
    Q_OBJECT

public:
    explicit AdministratorWidget(QWidget *parent = nullptr);
    ~AdministratorWidget();

    static AdministratorWidget*getInstance();      //获取单列对象
    void ConnectionServer();                      //主动进行服务的连接
    void releaseResource();                      //主动退出管理员

    //获取当前的配置项
    QString getServerip() const;
    quint16 getServerport() const;

protected:
    //重写关闭事件
    void closeEvent(QCloseEvent* event) override;
    //重写显示事件
    void showEvent(QShowEvent* event) override;

signals:
    // IP/端口改变时发出信号，其他模块监听
    void serverConfigChanged(const QString& ip, quint16 port);

private slots:
    void onServerClicked();
    void onConnectionServer(bool);      //当前连接服务器是否成功

public slots:
    void appendLog(const QString& log);           //外部调用：添加日志


private:
    void initUI();
    bool loadConfigFromFile(ServerConfig &outConfig); //读取服务器的配置文件
    bool saveConfigToFile(const ServerConfig &cfg);  //写入服务器的配置文件

private:
    Ui::AdministratorWidget *ui;
    ServerConfig m_serverConfig;    //当前的连接配置项
    QStringList m_logCache;        //日志的数据缓存
    QTimer* m_TimerServer;        //定时启动服务器的重连
};

#endif // ADMINISTRATORWIDGET_H
