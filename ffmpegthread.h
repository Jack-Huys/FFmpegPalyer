#ifndef FFMPEGTHREAD_H
#define FFMPEGTHREAD_H

#include "ffmpegCommon.h"
#include <QThread>
#include <QImage>
#include <QMutex>

namespace aos {

class ffmpegThread : public QThread
{
    Q_OBJECT

public:
    explicit ffmpegThread(QObject *parent = nullptr);

public:
    //初始化注册
    static void initlib();

protected:
    void run();

signals:
    void receiveImage(const QImage &image);

public slots:
    void setUrl(const QString &url);
    bool init();
    void free();
    void play();
    void pause();
    void next();
    void stop();

private:
    QString m_url;                    //视频流地址
    AVDictionary *m_options;          //参数对象

    AVFormatContext *m_formatCtx;     //视频容器格式上下文
    AVCodecContext *m_videoCodecCtx;  //视频解码器上下文
    AVCodecContext *m_audioCodecCtx;  //音频解码器上下文

    AVCodec *m_videoCodec;            //视频解码器
    AVCodec *m_audioCodec;            //音频解码器
    SwsContext *m_swsContext;         //处理图片数据对象


    int m_videoStreamIndex;           //视频流索引
    int m_audioStreamIndex;           //音频流索引
    int m_videoWidth;                 //视频宽度(计算视频分辨率 宽x高)
    int m_videoHeight;                //视频高度(计算视频分辨率 宽x高)

    AVPacket *m_avPacket;             //包对象
    AVFrame *m_yuvFrame;              //原始视频帧
    AVFrame *m_rgbFrame;              //转图片视频帧
    uint8_t *m_buffer;                //存储解码后图片buffer

    volatile bool m_stopped;          //线程停止标志位
    volatile bool m_isPlay;           //播放视频标志位
    int m_frameFinish;                //一帧完成

};


}


#endif // FFMPEGTHREAD_H
