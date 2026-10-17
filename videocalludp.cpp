#include "videocalludp.h"
#include <QDebug>
#include <QNetworkInterface>
# include <QVariant>
#include <QCamera>
# include <QTimer>
# include <QNetworkDatagram>


VideoCallUdp::VideoCallUdp(QObject *parent)
    : QObject(parent)
{


}

VideoCallUdp::~VideoCallUdp()
{
    stopCall();
    freeFfmpegResource();

    if (m_camera)
    {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }
}

bool VideoCallUdp::initCapture(int width, int height)
{
    if (m_initFinished)
    {
        qDebug() << "已完成初始化，无需重复initCapture";
        return true;
    }
    m_initFinished = true;

    m_camera = new QCamera(this);
    m_captureSession = new QMediaCaptureSession(this);
    m_videoSink = new QVideoSink(this);

    // 2. 正确绑定顺序
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSink);

    // 3. 帧回调连接
   bool connOk = connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, &VideoCallUdp::onCameraNewFrame, Qt::QueuedConnection);
    qDebug() << "videoFrameChanged connect 结果：" << connOk;

    // 监听摄像头异步错误（重点！捕获启动后异步报错）
    connect(m_camera, &QCamera::errorOccurred, this, [=](QCamera::Error err, const QString &msg){
        qWarning() << "摄像头异步错误 err:" << err << msg;
    });

    // 4. 启动摄像头
    m_camera->start();
    qDebug() << "调用start后 active:" << m_camera->isActive();

    if (m_camera->isActive())
    {
        qDebug() << "摄像头正在运行";
    }
    else
    {
        qDebug() << "摄像头未运行";
    }
    qDebug() << "camera error:" << m_camera->error() << m_camera->errorString();

    if (m_camera->error() != QCamera::NoError)
    {
        qDebug() << "摄像头启动失败:" << m_camera->errorString();
        delete m_camera;
        m_camera = nullptr;
        m_initFinished = false;
        return false;
    }

    if(!m_decCtx)
    {
        const AVCodec *decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
        if(!decoder)
        {
            qWarning() << "找不到H264解码器";
        }
        else
        {
            m_decCtx = avcodec_alloc_context3(decoder);
            // ⚠️ 注意：这里不要 avcodec_open2！缺少SPS/PPS会打开失败
        }
    }
    // 提前分配解码用的SwsContext，后续解码拿到分辨率再做参数设置
    /*
    if(!m_swsDec)
    {
        m_swsDec = sws_getContext(0,0,AV_PIX_FMT_NONE,
                                  0,0,AV_PIX_FMT_NONE,
                                  SWS_BILINEAR,nullptr,nullptr,nullptr);
    }
    */

    // !!! 这里不要写任何编码器、sws、解码器创建代码 !!!
    m_isCaptureInited = true;
    return true;
}
 //初始化端口号码的绑定
void VideoCallUdp::initPort()
{
    // 4 UDP绑定随机端口
    bool bindOk = m_udpSocket.bind(QHostAddress::AnyIPv4, 0);
    if (!bindOk)
    {
        qDebug() << "UDP bind失败:" << m_udpSocket.errorString();
        freeFfmpegResource();
        return ;
    }
    connect(&m_udpSocket, &QUdpSocket::readyRead,
            this, &VideoCallUdp::onUdpReadyRead, Qt::QueuedConnection);
    //m_udpSocket.writeDatagram("hello test", QHostAddress("127.0.0.1"), m_udpSocket.localPort());
    qDebug() << "VideoCapture init 成功, UDP端口:" << m_udpSocket.localPort();
}

void VideoCallUdp::startCapture()
{

    m_isSending = true;
    qDebug() << "开启视频发送";
}

void VideoCallUdp::stopCapture()
{
    m_isSending = false;
    qDebug() << "停止视频发送";
}

void VideoCallUdp::setRemoteAddr(const QString &ip, quint16 port)
{

    QHostAddress addr(ip);
    if (addr.isNull() || addr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol) {
        qDebug() << "无效的 IP 地址:" << ip;
        return ;
    }
    if (port == 0) {
        qDebug() << "端口号不能为 0";
        return ;
    }
    m_remoteAddr = addr;
    m_remotePort = port;
    qDebug () << "设置远端地址:" << ip << ":" << port;
}

void VideoCallUdp::stopCall()
{
    stopCapture();

    // -------- UDP处理【新增】--------
    // 断开readyRead信号，防止重复connect
    disconnect(&m_udpSocket, &QUdpSocket::readyRead,
               this, &VideoCallUdp::onUdpReadyRead);
    m_udpSocket.close(); //关闭socket，释放端口

    m_reassembleBuffer = FrameReassemble();
    m_txFrameSeq = 0;

    m_initFinished = false;
    m_isCaptureInited = false; //【修复：重置采集初始化标记】

    // 完整释放摄像头整套对象，彻底清理MFT资源
    if(m_camera)
    {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }
    if(m_captureSession)
    {
        delete m_captureSession;
        m_captureSession = nullptr;
    }
    if(m_videoSink)
    {
        delete m_videoSink;
        m_videoSink = nullptr;
    }

    m_remoteAddr = QHostAddress();
    m_remotePort = 0;
    m_swsDec = nullptr; // 重置解码sws上下文，让sws_getCachedContext重建

    freeFfmpegResource();
}

// 返回本机ip
QString VideoCallUdp::getLocalIp() const
{
    //return m_udpSocket.localAddress().toString();
     return getLocalLanIp();
    //return "127.0.0.1";
}

//返回本机的 端口
QString VideoCallUdp::getLocalPort() const
{
    QVariant port =  m_udpSocket.localPort();
    return port.toString();
}

void VideoCallUdp::freeFfmpegResource()
{
    if (m_spsTimer)
    {
        m_spsTimer->stop();
        delete m_spsTimer;
        m_spsTimer = nullptr;
    }

    if (m_swsEnc)
    {
        sws_freeContext(m_swsEnc);
        m_swsEnc = nullptr;
    }
    if (m_swsDec)
    {
        sws_freeContext(m_swsDec);
        m_swsDec = nullptr;
    }
    if (m_yuvEncFrame)
    {
        av_frame_free(&m_yuvEncFrame);
        m_yuvEncFrame = nullptr;
    }
    if (m_encCtx)
    {
        avcodec_free_context(&m_encCtx);
        m_encCtx = nullptr;
    }
    if (m_decCtx)
    {
        avcodec_free_context(&m_decCtx);
        m_decCtx = nullptr;
    }

    m_ffmpegInited = false;
    m_ffmpegFirstReady = false;
    m_framePts = 0;
}

void VideoCallUdp::onCameraNewFrame(const QVideoFrame &videoFrame)
{
    qDebug() << "收到摄像头帧";
    if(!m_isSending)
    {
        return; // 停止发送时直接丢弃
    }
    QImage frameImg = videoFrame.toImage();
    if (frameImg.isNull())
    {
        qDebug() << "frameImg 为空！";
        return;
    }
    // 修复：预乘Alpha图像不能直接送入FFmpeg
    if (frameImg.format() == QImage::Format_RGBA8888_Premultiplied)
    {
        frameImg = frameImg.convertToFormat(QImage::Format_RGBA8888);
    }
    emit sigLocalFrame(frameImg);

    // 第一帧到来，动态初始化全部FFmpeg资源
    if (!m_ffmpegInited)
    {
        m_videoW = frameImg.width();
        m_videoH = frameImg.height();
        qDebug() << "动态初始化FFmpeg，分辨率：" << m_videoW << "x" << m_videoH;
        // ========== H264 编码器初始化 ==========
        const AVCodec *encCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!encCodec)
        {
            qWarning() << "找不到H264编码器";
            return;
        }
        m_encCtx = avcodec_alloc_context3(encCodec);
        m_encCtx->width = m_videoW;
        m_encCtx->height = m_videoH;
        m_encCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        m_encCtx->time_base = {1, 30};
        m_encCtx->bit_rate = 1200000;
        m_encCtx->gop_size = 10;
        m_encCtx->keyint_min = 10;
        m_encCtx->max_b_frames = 0;
        //m_encCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        AVDictionary *opts = nullptr;
        av_dict_set(&opts, "preset", "ultrafast", 0);
        av_dict_set(&opts, "tune", "zerolatency", 0);
        // 只保留一条，加入annexb=1
        av_dict_set(&opts, "x264opts", "no-b-adapt=1:bframes=0:annexb=1", 0);
        int ret = avcodec_open2(m_encCtx, encCodec, &opts);
        av_dict_free(&opts);
        if (ret < 0)
        {
            qWarning() << "编码器打开失败";
            avcodec_free_context(&m_encCtx);
            m_encCtx = nullptr;
            return;
        }

        // 编码YUV缓存帧
        m_yuvEncFrame = av_frame_alloc();
        m_yuvEncFrame->width = m_videoW;
        m_yuvEncFrame->height = m_videoH;
        m_yuvEncFrame->format = AV_PIX_FMT_YUV420P;
        if (av_frame_get_buffer(m_yuvEncFrame, 1) < 0)
        {
            qWarning() << "av_frame_get_buffer 失败";
            av_frame_free(&m_yuvEncFrame);
            m_yuvEncFrame = nullptr;
            avcodec_free_context(&m_encCtx);
            m_encCtx = nullptr;
            return;
        }
        // RGBA -> YUV420P 转换器
        m_swsEnc = sws_getContext(
            m_videoW, m_videoH, AV_PIX_FMT_RGBA,
            m_videoW, m_videoH, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!m_swsEnc)
        {
            qWarning() << "swsEnc 创建失败";
            av_frame_free(&m_yuvEncFrame);
            m_yuvEncFrame = nullptr;
            avcodec_free_context(&m_encCtx);
            m_encCtx = nullptr;
            return;
        }

        // ========== 【已全部删除解码器、swsDec初始化，移到initCapture】 ==========

        m_ffmpegInited = true;
        m_ffmpegFirstReady = false;
        // 【修复】初始化完成立刻发送一次SPS/PPS
        if (m_encCtx && m_encCtx->extradata_size > 0)
        {
            QByteArray spsPpsData((char*)m_encCtx->extradata, m_encCtx->extradata_size);
            sendRawUdpPacket(spsPpsData);
            qDebug() << "初始化完成，立即发送SPS/PPS";
        }
        // 定时发送SPS/PPS
        m_spsTimer = new QTimer(this);
        connect(m_spsTimer, &QTimer::timeout, this, [this]()
                {
                    if (m_encCtx && m_encCtx->extradata_size > 0)
                    {
                        QByteArray spsPpsData((char*)m_encCtx->extradata, m_encCtx->extradata_size);
                        sendRawUdpPacket(spsPpsData);
                    }
                });
        m_spsTimer->start(2000);
    }
    // 未初始化完成直接丢弃帧
    if (!m_ffmpegInited || !m_swsEnc || !m_encCtx || !m_yuvEncFrame)
    {
        return;
    }
    if (!m_ffmpegFirstReady)
    {
        m_ffmpegFirstReady = true;
        return;
    }
    // RGBA图像转YUV420P
    uint8_t* srcPlanes[1] = { frameImg.bits() };
    int srcStrides[1] = { static_cast<int>(frameImg.bytesPerLine()) };
    sws_scale(m_swsEnc,
              srcPlanes,
              srcStrides,
              0,
              m_videoH,
              m_yuvEncFrame->data,
              m_yuvEncFrame->linesize);
    m_yuvEncFrame->pts = m_framePts++;
    int sendRet = avcodec_send_frame(m_encCtx, m_yuvEncFrame);
    //qDebug() << "avcodec_send_frame ret =" << sendRet;
    if (sendRet < 0)
    {
        qWarning() << "avcodec_send_frame 失败";
        return;
    }
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    while (avcodec_receive_packet(m_encCtx, &pkt) == 0)
    {
        //qDebug() << "输出NALU，长度 =" << pkt.size;
        // 【核心修复】不再直接调用sendRawUdpPacket，统一走分片封装函数
        encodeAndSendFrame(&pkt);
        av_packet_unref(&pkt);
    }
}



void VideoCallUdp::encodeAndSendFrame(AVPacket *pkt)
{
    if(!pkt || m_remoteAddr.isNull())
        return;

    bool isIDRFrame = (pkt->flags & AV_PKT_FLAG_KEY);
    if(isIDRFrame)
    {
        if(m_encCtx->extradata && m_encCtx->extradata_size > 0)
        {
            QByteArray spsPpsData((char*)m_encCtx->extradata, m_encCtx->extradata_size);
            sendRawUdpPacket(spsPpsData);
            //qDebug() << "I帧触发，补发SPS/PPS";
        }
    }

    int totalLen = pkt->size;
    int offset = 0;
    int sliceCount = (totalLen + UDP_MTU - 1) / UDP_MTU;

    for (int i = 0; i < sliceCount; i++)
    {
        int sliceLen = qMin(UDP_MTU, totalLen - offset);

        UdpFrameHeader hdr;
        hdr.magic = UDP_VIDEO_MAGIC;
        hdr.frameSeq = m_txFrameSeq;
        hdr.sliceIdx = i;
        hdr.sliceTotal = sliceCount;

        QByteArray udpPacket;
        udpPacket.append((char *)&hdr, sizeof(UdpFrameHeader));
        udpPacket.append((char *)pkt->data + offset, sliceLen);

        qDebug() << "[UDP发送] 目标:" << m_remoteAddr.toString() << ":" << m_remotePort
                 << " 包大小:" << udpPacket.size();
        qint64 sent = m_udpSocket.writeDatagram(udpPacket, m_remoteAddr, m_remotePort);
        qDebug() << "[UDP发送] 返回字节数=" << sent;

        offset += sliceLen;
    }
    m_txFrameSeq++;
}

// 新增工具函数：直接发送原始UDP小包（用于SPS/PPS）
void VideoCallUdp::sendRawUdpPacket(const QByteArray& data)
{
    // 远端地址未设置，直接丢弃
    if (m_remoteAddr.isNull())
    {
        qDebug() << "sendRawUdpPacket 远端地址为空，丢弃";
        return;
    }
    // SPS/PPS独立数据包，不分片
    UdpFrameHeader hdr;
    hdr.magic = UDP_VIDEO_MAGIC;
    hdr.frameSeq = m_txFrameSeq;
    hdr.sliceIdx = 0;
    hdr.sliceTotal = 1;

    QByteArray udpPacket;
    udpPacket.append((char *)&hdr, sizeof(UdpFrameHeader));
    udpPacket.append(data);
    qDebug() << "[发送SPS/PPS] 大小:" << udpPacket.size();
    m_udpSocket.writeDatagram(udpPacket, m_remoteAddr, m_remotePort);
}

void VideoCallUdp::onUdpReadyRead()
{
    qDebug() << "-------  收到UDP数据包 ---------";
    qDebug() << "当前是否已初始化采集/解码：" << m_isCaptureInited;

    while (m_udpSocket.hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_udpSocket.receiveDatagram();
        if (!datagram.isValid())
            continue;

        QByteArray data = datagram.data();
        processUdpDatagram(data);
    }
}


void VideoCallUdp::processUdpDatagram(const QByteArray &data)
{
    if (data.size() < sizeof(UdpFrameHeader))
        return;
    UdpFrameHeader hdr;
    memcpy(&hdr, data.constData(), sizeof(UdpFrameHeader));
    if (hdr.magic != UDP_VIDEO_MAGIC)
        return; // 非法数据包直接丢弃
    QByteArray payload = data.mid(sizeof(UdpFrameHeader));
    // 新帧序号，清空旧缓存
    if (hdr.frameSeq != m_reassembleBuffer.frameSeq)
    {
        m_reassembleBuffer.frameSeq = hdr.frameSeq;
        m_reassembleBuffer.totalSlice = hdr.sliceTotal;
        m_reassembleBuffer.slices.clear();
    }
    // 存入分片
    m_reassembleBuffer.slices.insert(hdr.sliceIdx, payload);
    // 判断是否集齐所有分片
    if (m_reassembleBuffer.slices.size() == m_reassembleBuffer.totalSlice)
    {
        QByteArray fullNalu;
        for (auto &slice : m_reassembleBuffer.slices.values())
        {
            fullNalu.append(slice);
        }

        // 【兼容旧FFmpeg，弃用av_packet_from_data】堆内存拷贝
        uint8_t* heapBuf = (uint8_t*)av_malloc(fullNalu.size());
        if(heapBuf)
        {
            memcpy(heapBuf, fullNalu.constData(), fullNalu.size());

            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = heapBuf;
            pkt.size = fullNalu.size();

            decodeH264Packet(&pkt);

            av_free(heapBuf); // 使用完立刻释放堆内存
        }

        // 清空缓存准备下一帧
        m_reassembleBuffer.slices.clear();
    }
}



void VideoCallUdp::decodeH264Packet(const AVPacket *pkt)
{
    if(!m_decCtx)
    {
        qDebug() << "解码器上下文未分配，丢弃包";
        return;
    }
    // 解码器还没有打开，尝试从数据包加载SPS/PPS并open
    if(!avcodec_is_open(m_decCtx))
    {
        if(pkt->size <= 0)
            return;

        // 【修复】释放旧extradata，防止内存泄漏/野指针
        av_freep(&m_decCtx->extradata);
        m_decCtx->extradata_size = 0;

        m_decCtx->extradata = (uint8_t*)av_mallocz(pkt->size + AV_INPUT_BUFFER_PADDING_SIZE);
        if(!m_decCtx->extradata)
        {
            qWarning() << "av_mallocz extradata 内存分配失败";
            return;
        }
        memcpy(m_decCtx->extradata, pkt->data, pkt->size);

        int retOpen = avcodec_open2(m_decCtx, m_decCtx->codec, nullptr);
        if(retOpen < 0)
        {
            qWarning() << "avcodec_open2 解码器打开失败";
            av_freep(&m_decCtx->extradata);
            m_decCtx->extradata_size = 0;
            return;
        }
        qDebug() << "H264解码器open成功";
        // SPS/PPS仅用于初始化解码器，不要送入avcodec_send_packet
        return;
    }

    int ret = avcodec_send_packet(m_decCtx, pkt);
    if (ret < 0)
    {
        qWarning() << "avcodec_send_packet failed ret=" << ret;
        return;
    }

    AVFrame* yuvFrame = av_frame_alloc();
    if(!yuvFrame)
    {
        qWarning() << "av_frame_alloc 失败";
        return;
    }

    while (avcodec_receive_frame(m_decCtx, yuvFrame) == 0)
    {
        qDebug() << "成功解码图像帧";
        int realW = yuvFrame->width;
        int realH = yuvFrame->height;
        if(realW <=0 || realH <=0)
            continue;

        m_swsDec = sws_getCachedContext(m_swsDec,
                                        realW, realH, (AVPixelFormat)yuvFrame->format,
                                        realW, realH, AV_PIX_FMT_BGR24,
                                        SWS_BILINEAR, nullptr, nullptr, nullptr);

        if(!m_swsDec)
        {
            qWarning() << "sws_getCachedContext 失败";
            continue;
        }

        // ==========【修复开始】==========
        // 1. 分配紧密排列BGR24缓冲区（无padding）
        int rawStride = realW * 3;
        uint8_t* rawBuf = new uint8_t[rawStride * realH];
        uint8_t* dstPlanes[1] = { rawBuf };
        int dstStrides[1] = { rawStride };

        sws_scale(m_swsDec,
                  yuvFrame->data,
                  yuvFrame->linesize,
                  0,
                  realH,
                  dstPlanes,
                  dstStrides);

        // 2. 从原始内存构造QImage（QImage共享内存，最后必须拷贝！）
        QImage tempImg(rawBuf, realW, realH, rawStride, QImage::Format_BGR888);
        QImage dstImg = tempImg.copy(); // 拷贝，脱离rawBuf生命周期
        delete [] rawBuf;
        // ==========【修复结束】==========

        emit sigRemoteVideoFrame(dstImg);
    }


    av_frame_free(&yuvFrame);
}

//获取本机第一个有效的局域网IPv4，获取失败返回 "127.0.0.1"
QString VideoCallUdp::getLocalLanIp() const
{
    QString defaultIp = "127.0.0.1";
    const QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    for(const QNetworkInterface& iface : ifaces)
    {
        // 过滤：网卡无效、没有启用、回环网卡
        if(!iface.isValid()
            || !iface.flags().testFlag(QNetworkInterface::IsUp)
            || iface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            continue;
        }

        // type() == Virtual 代表虚拟网卡（Qt5.11+才支持type接口）
        if(iface.type() == QNetworkInterface::Virtual)
        {
            continue;
        }

        const QList<QNetworkAddressEntry> addrs = iface.addressEntries();
        for(const QNetworkAddressEntry& entry : addrs)
        {
            QHostAddress ip = entry.ip();
            // 只取IPv4
            if(ip.protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            QString ipStr = ip.toString();
            if(ipStr == "127.0.0.1")
                continue;

            // 简单过滤内网网段：192.168.x.x / 10.x.x.x /172.16‑172.31
            QHostAddress addr(ipStr);
            quint32 v4 = addr.toIPv4Address();
            quint8 a = (v4 >> 24) & 0xFF;
            quint8 b = (v4 >> 16) & 0xFF;

            bool isLan =
                (a == 192 && b == 168)
                || (a == 10)
                || (a == 172 && (b >=16 && b <=31));

            if(isLan)
            {
                qDebug()<<"找到局域网IP:"<<ipStr;
                return ipStr;
            }
        }
    }
    return defaultIp;


}

