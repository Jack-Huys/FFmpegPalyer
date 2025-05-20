#include "ffmpegthread.h"


#include <QMutex>
#include <QImage>

aos::ffmpegThread::ffmpegThread(QObject *parent) : QThread(parent)
{
    setObjectName("FFmpegThread");
    m_url = "";
    m_options = nullptr;
    m_formatCtx = nullptr;
    m_videoCodecCtx = nullptr;

    m_audioCodecCtx = nullptr;
    m_videoCodec = nullptr;
    m_audioCodec = nullptr;
    m_swsContext = nullptr;

    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
    m_videoWidth = 0;
    m_videoHeight = 0;

    m_avPacket = nullptr;
    m_yuvFrame = nullptr;
    m_rgbFrame = nullptr;
    m_buffer = nullptr;

    m_stopped = false;
    m_isPlay = false;

    //初始化注册,一个软件中只注册一次即可
    ffmpegThread::initlib();
}

void aos::ffmpegThread::initlib()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static bool isInit = false;
    if(!isInit)
    {
        //注册所有的音视频解码器和格式，以及网络流格式初始化
        av_register_all();
        avformat_network_init();
        isInit = true;
        qDebug() << TIMEMS << "init ffmpeg lib ok" << " version:" << FFMPEG_VERSION;
    }

}

void aos::ffmpegThread::run()
{
    int64_t startTime = av_gettime();
    while(!m_stopped)
    {
        if(m_isPlay)
        {
            this->init();
            m_isPlay = false;
            continue;
        }
        m_frameFinish = av_read_frame(m_formatCtx, m_avPacket);
        if(m_frameFinish >= 0)
        {
            //设置倍速播放
            double speed = 1;
            m_avPacket->pts = m_avPacket->pts / speed;
            m_avPacket->dts = m_avPacket->dts / speed;

            int index = m_avPacket->stream_index;
            if(index == m_videoStreamIndex)
            {
                //解码视频流
                m_frameFinish = avcodec_send_packet(m_videoCodecCtx, m_avPacket);
                if (m_frameFinish < 0) {
                    continue;
                }

                m_frameFinish = avcodec_receive_frame(m_videoCodecCtx, m_yuvFrame);
                if (m_frameFinish < 0) {
                    continue;
                }

                if (m_frameFinish >= 0) {
                    //将数据转成一张图片
                    sws_scale(m_swsContext, (const uint8_t *const *)m_yuvFrame->data, m_yuvFrame->linesize, 0, m_videoHeight, m_rgbFrame->data, m_rgbFrame->linesize);

                    //以下两种方法都可以
                    //QImage image(rgbFrame->data[0], videoWidth, videoHeight, QImage::Format_RGB32);
                    QImage image((uchar *)m_buffer, m_videoWidth, m_videoHeight, QImage::Format_RGB32);
                    if (!image.isNull()) {
                        emit receiveImage(image);
                    }
                    usleep(1);
                }
                //延时(不然文件会立即全部播放完)
                AVRational timeBase = {1, AV_TIME_BASE};
                int64_t ptsTime = av_rescale_q(m_avPacket->dts, m_formatCtx->streams[m_videoStreamIndex]->time_base, timeBase);
                int64_t nowTime = av_gettime() - startTime;
                if (ptsTime > nowTime) {
                    av_usleep(ptsTime - nowTime);
                }

            }
            else{
                //音频流解析
            }
        }
        av_packet_unref(m_avPacket);
        av_freep(m_avPacket);
        usleep(1);
    }
    //线程结束后释放资源
    free();
    m_stopped = false;
    m_isPlay = false;
    qDebug() << TIMEMS << "stop ffmpeg thread";
}

void aos::ffmpegThread::setUrl(const QString &url)
{
    m_url = url;
}

bool aos::ffmpegThread::init()
{
    //////////////////////////////////////////////////////////
    ////////打开码流前指定参数///////////////////////////////////

    //在打开码流前指定各种参数比如:探测时间/超时时间/最大延时等
    //可以做成配置参数或显示

    //设置缓存大小,1080p可将值调大
    av_dict_set(&m_options, "buffer_size", "8192000", 0);

    //以tcp方式打开,如果以udp方式打开将tcp替换为udp
    av_dict_set(&m_options, "rtsp_transport", "tcp", 0);

    //设置超时断开连接时间,单位微秒,3000000表示3秒
    av_dict_set(&m_options, "stimeout", "3000000", 0);

    //设置最大时延,单位微秒,1000000表示1秒
    av_dict_set(&m_options, "max_delay", "1000000", 0);

    //自动开启线程数
    av_dict_set(&m_options, "threads", "auto", 0);


    m_formatCtx = avformat_alloc_context();
    int result = avformat_open_input(&m_formatCtx, m_url.toStdString().c_str(), nullptr, &m_options);
    if(result < 0)
    {
        qDebug() << TIMEMS << "open input error" << m_url;
        return false;
    }

    //释放设置参数
    if (m_options != nullptr) {
        av_dict_free(&m_options);
    }

    //获取流信息
    result = avformat_find_stream_info(m_formatCtx, nullptr);
    if (result < 0) {
        qDebug() << TIMEMS << "find stream info error";
        return false;
    }

    //////////////////////////////////////////////////////////
    ////////视频流/////////////////////////////////////////////

    m_videoStreamIndex = av_find_best_stream(m_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &m_videoCodec, 0);
    if (m_videoStreamIndex < 0) {
        qDebug() << TIMEMS << "find video stream index error";
        return false;
    }
    //获取视频流
    AVStream *videoStream = m_formatCtx->streams[m_videoStreamIndex];


    //通过解码器id获取视频流解码器,或者指定解码器
    m_videoCodecCtx = videoStream->codec;
    m_videoCodec = avcodec_find_decoder(m_videoCodecCtx->codec_id);
    //videoCodec = avcodec_find_decoder_by_name("h264_qsv");
    if (m_videoCodec == nullptr) {
        qDebug() << TIMEMS << "video decoder not found";
        return false;
    }

    //设置加速解码
//    m_videoCodecCtx->lowres = m_videoCodec->max_lowres;
//    m_videoCodecCtx->flags2 |= AV_CODEC_FLAG2_FAST;

    //打开视频解码器
    result = avcodec_open2(m_videoCodecCtx, m_videoCodec, nullptr);
    if (result < 0) {
        qDebug() << TIMEMS << "open video codec error";
        return false;
    }

    //获取分辨率大小
    m_videoWidth = videoStream->codec->width;
    m_videoHeight = videoStream->codec->height;

    //如果没有获取到宽高则返回
    if (m_videoWidth == 0 || m_videoHeight == 0) {
        qDebug() << TIMEMS << "find width height error";
        return false;
    }

    QString videoInfo = QString("视频流信息 -> 索引: %1  解码: %2  格式: %3  时长: %4 秒  分辨率: %5*%6")
                        .arg(m_videoStreamIndex).arg(m_videoCodec->name).arg(m_formatCtx->iformat->name)
                        .arg((m_formatCtx->duration) / 1000000).arg(m_videoWidth).arg(m_videoHeight);
    qDebug() << TIMEMS << videoInfo;

    //////////////////////////////////////////////////////////
    ////////音频流/////////////////////////////////////////////

    m_audioStreamIndex = -1;
    //查找音频流索引 可能视频本身就没有音频
    for (uint i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
            break;
        }
    }

    if (m_audioStreamIndex == -1) {
        qDebug() << TIMEMS << "find audio stream index error";
    } else {
        //获取音频流解码器
        AVStream *audioStream = m_formatCtx->streams[m_audioStreamIndex];
        m_audioCodecCtx = audioStream->codec;
        m_audioCodec = avcodec_find_decoder(m_audioCodecCtx->codec_id);
        //audioCodec = avcodec_find_decoder_by_name("aac");
        if (m_audioCodec == nullptr) {
            qDebug() << TIMEMS << "audio codec not found";
            return false;
        }
        //打开音频解码器
        result = avcodec_open2(m_audioCodecCtx, m_audioCodec, nullptr);
        if (result < 0) {
            qDebug() << TIMEMS << "open audio codec error";
            return false;
        }

        QString audioInfo = QString("音频流信息 -> 索引: %1  解码: %2  比特率: %3  声道数: %4  采样: %5")
                            .arg(m_audioStreamIndex).arg(m_audioCodec->name).arg(m_formatCtx->bit_rate)
                            .arg(m_audioCodecCtx->channels).arg(m_audioCodecCtx->sample_rate);
        qDebug() << TIMEMS << audioInfo;

    }
    //////////////////////////////////////////////////////////

    //预分配好内存
    m_avPacket = av_packet_alloc();
    m_yuvFrame = av_frame_alloc();
    m_rgbFrame = av_frame_alloc();

    int byte = avpicture_get_size(AV_PIX_FMT_RGB32, m_videoWidth, m_videoHeight);
    m_buffer = (uint8_t *)av_malloc(byte * sizeof(uint8_t));

    //定义像素格式
    AVPixelFormat srcFormat = AV_PIX_FMT_YUV420P;
    AVPixelFormat dstFormat = AV_PIX_FMT_RGB32;
    //通过解码器获取解码格式
    srcFormat = m_videoCodecCtx->pix_fmt;
    //默认最快速度的解码采用的SWS_FAST_BILINEAR参数,可能会丢失部分图片数据,可以自行更改成其他参数
    int flags = SWS_FAST_BILINEAR;

    //开辟缓存存储一帧数据
    //以下两种方法都可以,avpicture_fill已经逐渐被废弃
    //avpicture_fill((AVPicture *)m_rgbFrame, m_buffer, dstFormat, m_videoWidth, m_videoHeight);
    av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize, m_buffer, dstFormat, m_videoWidth, m_videoHeight, 1);

    //图像转换
    m_swsContext = sws_getContext(m_videoWidth, m_videoHeight, srcFormat, m_videoWidth, m_videoHeight, dstFormat, flags, nullptr, nullptr, nullptr);

    qDebug() << TIMEMS << "init ffmpeg finsh";

    return true;
}

void aos::ffmpegThread::free()
{
    if (m_swsContext != nullptr) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }

    if (m_avPacket != nullptr) {
        av_packet_unref(m_avPacket);
        m_avPacket = nullptr;
    }

    if (m_yuvFrame != nullptr) {
        av_frame_free(&m_yuvFrame);
        m_yuvFrame = nullptr;
    }

    if (m_rgbFrame != nullptr) {
        av_frame_free(&m_rgbFrame);
        m_rgbFrame = nullptr;
    }

    if (m_videoCodecCtx != nullptr) {
        avcodec_close(m_videoCodecCtx);
        m_videoCodecCtx = nullptr;
    }

    if (m_audioCodecCtx != nullptr) {
        avcodec_close(m_audioCodecCtx);
        m_audioCodecCtx = nullptr;
    }

    if (m_formatCtx != nullptr) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
    }

    if(m_options != nullptr){
        av_dict_free(&m_options);
        m_options = nullptr;
    }

    if(m_buffer)
    {
        av_free(m_buffer);
        m_buffer = nullptr;
    }

}

void aos::ffmpegThread::play()
{
    m_isPlay = true;
}

void aos::ffmpegThread::pause()
{

}

void aos::ffmpegThread::next()
{

}

void aos::ffmpegThread::stop()
{
    m_stopped = true;
}
