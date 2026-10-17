#ifndef VIDEOCALLUDP_H
#define VIDEOCALLUDP_H

#include <QObject>
#include <QUdpSocket>
#include <QCamera>
#include <QVideoSink>
#include <QVideoFrame>
# include <QMap>
#include <QMediaCaptureSession>
# include <QTimer>
# include <QPaintEvent>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

// UDP分片最大载荷（MTU安全值）
constexpr int UDP_MTU = 16000;
// 数据包魔数
constexpr quint32 UDP_VIDEO_MAGIC = 0xAABBCCDD;

// UDP视频包头（固定大小！不能调整成员顺序）
struct UdpFrameHeader{
    quint32 magic;
    quint32 frameSeq;
    quint16 sliceIdx;
    quint16 sliceTotal;
};

// 分片重组缓存结构
struct FrameReassemble{
    quint32 frameSeq = 0;
    quint16 totalSlice = 0;
    QMap<int, QByteArray> slices;
};

class VideoCallUdp : public QObject
{
    Q_OBJECT

public:
    explicit VideoCallUdp(QObject *parent = nullptr);
    ~VideoCallUdp() override;

    // 【一次性初始化】摄像头、编码器、解码器、UDP
    bool initCapture(int width = 640, int height = 480);

    //初始化端口号码的绑定
    void initPort();

    // 开始向外发送视频流（通话接通后调用）
    void startCapture();
    // 停止向外发送视频流
    void stopCapture();

    // 设置对方远端UDP地址
    void setRemoteAddr(const QString &ip, quint16 port);

    // 挂断通话：停止推流，清空重组缓存
    void stopCall();

    // 新增工具函数：直接发送原始UDP小包（用于SPS/PPS）
    void sendRawUdpPacket(const QByteArray& data);

    // 获取本机UDP监听信息
    QString getLocalIp() const;
    QString getLocalPort() const;

signals:
    // 解码得到远端画面，抛给UI渲染
    void sigRemoteVideoFrame(const QImage &img);
    // 本地摄像头原始帧，向外抛出用于本地预览
    void sigLocalFrame(const QImage& img);

private slots:
    // 摄像头新帧回调
    void onCameraNewFrame(const QVideoFrame &frame);
    // UDP收到远端数据包
    void onUdpReadyRead();

private:
    // FFmpeg 资源释放统一封装
    void freeFfmpegResource();

    // 本地编码发送逻辑
    void encodeAndSendFrame(AVPacket *pkt);

    // UDP数据包接收、分片重组、解码
    void processUdpDatagram(const QByteArray &data);
    // 完整H264 AVPacket解码
    void decodeH264Packet(const AVPacket *pkt);

    //获取本机第一个有效的局域网IPv4，获取失败返回 "127.0.0.1"
    QString getLocalLanIp() const;

private:
    // 摄像头相关
    QCamera *m_camera = nullptr;
    QMediaCaptureSession* m_captureSession = nullptr;
    bool m_initFinished = false;
    bool m_isSending = false;      // 是否允许向外发送视频
    QTimer* m_spsTimer;             //定时器

    QVideoSink* m_videoSink = nullptr;

    // FFmpeg 编码（本地摄像头 -> H264）
    AVCodecContext *m_encCtx = nullptr;
    AVFrame *m_yuvEncFrame = nullptr;

    // FFmpeg 解码（远端H264 -> RGB图像）
    AVCodecContext *m_decCtx = nullptr;
    SwsContext *m_swsEnc = nullptr;
    SwsContext *m_swsDec = nullptr;

    // UDP
    QUdpSocket m_udpSocket;
    QHostAddress m_remoteAddr;
    quint16 m_remotePort = 0;

    quint32 m_txFrameSeq = 0;               // 发送帧序号
    FrameReassemble m_reassembleBuffer;     // 分片重组缓冲区

    int m_videoW = 640;
    int m_videoH = 480;
private:
    bool m_ffmpegInited = false; // 新增：FFmpeg延迟初始化标记
    bool m_ffmpegFirstReady = false;
    int64_t m_framePts = 0;   // 新增PTS计数器

    bool m_isCaptureInited = false;//判断是否初始化完成
};


#endif
