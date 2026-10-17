#include "administratorwidget.h"
#include "ui_administratorwidget.h"

#include <QDateTime>
#include <QScrollBar>
# include <QMessageBox>


AdministratorWidget::AdministratorWidget(QWidget *parent)
    : BasicWindow(parent)
    , ui(new Ui::AdministratorWidget)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->initTitleBar();
    this->setTitleBarTitle("", ":/Resources/MainWindow/qqlogoclassic.png");

    //【程序启动加载配置】
    loadConfigFromFile(m_serverConfig);
    //填充UI输入框
    ui->InputIP->setText(m_serverConfig.ip);
    ui->Inputport->setText(QString::number(m_serverConfig.port));

    //日志控件设置只读，限制最大行数防止内存暴涨
    ui->plainTextEdit->setReadOnly(true);
    ui->plainTextEdit->setMaximumBlockCount(1200);

    //绑定保存配置按键
    connect(ui->pushButton, &QPushButton::clicked,
            this, &AdministratorWidget::onServerClicked);
    //判断当前是否连接成功服务器
    connect(TcpClient::getInstance(), &TcpClient::SuccessServer,
            this, &AdministratorWidget::onConnectionServer);

    m_TimerServer = new QTimer(this) ;
    m_TimerServer->setInterval(6000);
    //当断开与服务器的连接时每3秒主动请求连接服务器
    connect(m_TimerServer, &QTimer::timeout,
            this, &AdministratorWidget::ConnectionServer);

}

AdministratorWidget::~AdministratorWidget()
{
    delete ui;
}

  //获取单列对象
AdministratorWidget *AdministratorWidget::getInstance()
{

    static AdministratorWidget instance;
    return& instance;

}

 //主动进行服务的连接
void AdministratorWidget::ConnectionServer()
{

    const QString ip = ui->InputIP->text();
    const quint16 port = ui->Inputport->text().toUShort();

    QString str = "当前的ip: " + ip + "端口: " + QString::number(port) ;
    qDebug() << str ;

    if(ip.isEmpty()){
        appendLog("连接属性错误：IP地址不能为空！");
        qDebug()<<"连接属性错误：IP地址不能为空！";
        QMessageBox::information(this,
                                "连接服务器",
                                 "连接属性错误：IP地址不能为空！");
        return;
    }else if(port < 1 || port > 65535){
        appendLog("连接属性错误：端口必须在 1 ~ 65535 范围！");
        qDebug()<<"连接属性错误：端口必须在 1 ~ 65535 范围！！";
        QMessageBox::information(this,
                                 "连接服务器",
                                 "连接属性错误：端口必须在 1 ~ 65535 范围！");
        return;
    }

    emit serverConfigChanged(ip, port);
}

//主动退出管理员
void AdministratorWidget::releaseResource()
{
    //停止重连定时器
    if(m_TimerServer)
    {
        if(m_TimerServer->isActive())
            m_TimerServer->stop();
        // 因为new的时候父对象是this，不需要手动delete，Qt父对象树自动回收
    }

    this->close();
    m_logCache.clear();
    if(ui && ui->plainTextEdit)
    {
        ui->plainTextEdit->clear();
    }
    disconnect(this);

}

//外部调用：添加日志
void AdministratorWidget::appendLog(const QString &log)
{

    // 自动加时间戳
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString fullText = QString("[%1]：%2").arg(time).arg(log);

    // 判断窗口是否可见，隐藏状态写入缓存，不操作UI控件
    if(!isVisible())
    {
        m_logCache.append(fullText);
        //限制缓存最大行数，防止内存暴涨
        if(m_logCache.size() > 1500)
        {
            m_logCache.removeFirst();
        }
        return;
    }
    ui->plainTextEdit->appendPlainText(fullText);
    //自动滚动到日志最底部
    ui->plainTextEdit->moveCursor(QTextCursor::End);
}

QString AdministratorWidget::getServerip() const
{
    return ui->InputIP->text();

}

quint16 AdministratorWidget::getServerport() const
{
    return ui->Inputport->text().toUShort();
}

 //重写关闭事件
void AdministratorWidget::closeEvent(QCloseEvent *event)
{

    hide();
    event->ignore();
}

//重写显示事件
void AdministratorWidget::showEvent(QShowEvent *event)
{
    //调用父类事件
    BasicWindow::showEvent(event);

    //把缓存全部输出到plainTextEdit
    for(const QString& line : m_logCache)
    {
        ui->plainTextEdit->appendPlainText(line);
    }
    //缓存刷完清空
    m_logCache.clear();

    //光标滚动到底部
    ui->plainTextEdit->moveCursor(QTextCursor::End);

}

//点击 保存配置按键时
void AdministratorWidget::onServerClicked()
{
    // 从【输入框】拿取数据，不要拿Label！
    const QString ip = ui->InputIP->text().trimmed();
    // 注意控件名字：确认ui文件输入框名字是 InputPort（P大写）
    const quint16 port = ui->Inputport->text().toUShort();

    ServerConfig msg;
    msg.ip = ip;
    msg.port = port;

    if(msg.ip.isEmpty())
    {
        appendLog("错误：IP地址不能为空！");
        return;
    }
    if(msg.port < 1 || msg.port > 65535)
    {
        appendLog("错误：端口必须在 1 ~ 65535 范围！");
        return;
    }

    bool ret = saveConfigToFile(msg);
    if(ret)
    {
        m_serverConfig = msg;
        // ！！emit一定要传校验过的msg.port，不要传label读取出来的局部port！
        emit serverConfigChanged(msg.ip, msg.port);
        appendLog(QString("已经修改服务器配置 ip: %1 端口: %2")
                      .arg(msg.ip)
                      .arg(msg.port));
    }
    else
    {
        appendLog("保存服务器配置文件失败！");
    }

}

 //当前连接服务器是否成功
void AdministratorWidget::onConnectionServer(bool succecc)
{

    if (!succecc) {
        QMessageBox::information(this,
                                 "提示"
                                 ,"服务器连接失败");
        appendLog("服务器连接失败");
        m_TimerServer->start();//启动定时器任务
        return ;
    }
    else {
        appendLog("服务器连接成功");
        // ✅连接成功，立刻关闭重连定时器
        if(m_TimerServer->isActive())
        {
            m_TimerServer->stop();
        }
        return ;
    }
}

void AdministratorWidget::initUI()
{

}

//读取服务器的配置文件
bool AdministratorWidget::loadConfigFromFile(ServerConfig &outConfig)
{
    const QString FileURL = QPeratioFile::instance()->getServerConfiguration();

    QFile file(FileURL);
    if(!file.exists()){
        appendLog("配置文件ServerConfig.json不存在，使用默认IP:127.0.0.1 端口:8899");
        return false;
    }


    if(!file.open(QFile::ReadOnly)){
        appendLog(QString("读取配置文件失败：%1").arg(file.errorString()));
        return false;
    }

    QByteArray rawData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if(doc.isNull() || !doc.isObject())
    {
        appendLog("配置文件JSON解析失败，采用默认参数");
        return false;
    }

    QJsonObject obj = doc.object();
    //当配置文件没值的时候，则使用默认的ip 和 端口
    outConfig.ip = obj.value("server_ip").toString("127.0.0.1");
    int tempPort = obj.value("server_port").toInt(8899);

    //端口合法性校验
    if(tempPort >=1 && tempPort <=65535)
    {
        outConfig.port = static_cast<quint16>(tempPort);
    }
    else
    {
        outConfig.port = 8899;
        appendLog("配置文件端口非法，重置默认8899");
    }
    appendLog(QString("成功加载配置：%1 : %2").arg(outConfig.ip).arg(outConfig.port));
    qDebug()<< QString("成功加载配置：%1 : %2").arg(outConfig.ip).arg(outConfig.port);
    return true;

}

//写入服务器的配置文件
bool AdministratorWidget::saveConfigToFile(const ServerConfig &cfg)
{
    const QString FileURL = QPeratioFile::instance()->getServerConfiguration();

    QFile file(FileURL);
    //Truncate：覆盖原有全部内容
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        appendLog(QString("写入配置文件失败：%1").arg(file.errorString()));
        return false;
    }

    QJsonObject jsonObj;
    jsonObj.insert("server_ip", cfg.ip);
    jsonObj.insert("server_port", static_cast<int>(cfg.port));

    QJsonDocument doc(jsonObj);
    //Indented格式化输出，方便人打开文件查看
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    appendLog(QString("配置保存成功 IP=%1 Port=%2").arg(cfg.ip).arg(cfg.port));
    return true;

}
