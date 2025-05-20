#pragma execution_character_set("utf-8")

#include "mainwindow.h"

#include <QApplication>
// #include <QTextCodec>

#include <iostream>

// decode packets into frames
static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);
// save a frame into a .pgm file
static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);

static int test01()
{
    std::string videoPath = "small_bunny_1080p_60fps.mp4";

    //注册库中所有可用的文件格式和编解码器
    av_register_all();

    //初始化网络流格式,使用网络流时必须先执行
    avformat_network_init();

    //格式上下文
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
        std::cout << "ERROR could not allocate memory for Format Context\n";
        return -1;
    }

    if(avformat_open_input(&pFormatContext, videoPath.c_str(), nullptr, nullptr)!=0)
    {
        std::cout << "无法打开文件\n";
        return -1;
    }
    printf("Format %s, duration %lld us", pFormatContext->iformat->long_name, pFormatContext->duration);

    //获取流信息
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0)
    {
         std::cout << "ERROR could not get the stream info\n";
         return -1;
    }

    AVCodec *pCodec = nullptr;
   // https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
   AVCodecParameters *pCodecParameters =  nullptr;
   int video_stream_index = -1;  //视频流的索引
   //获取所有流信息
   for (int i = 0; i < pFormatContext->nb_streams; i++)
   {
        AVCodecParameters *pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        //获取流对应的解码器
        AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        // 用于视频和音频
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
          std::cout << "Video Codec: resolution" << pLocalCodecParameters->width << " x " <<pLocalCodecParameters->height << std::endl;
          if (video_stream_index == -1)
          {
              video_stream_index = i;
              pCodec = pLocalCodec;
              pCodecParameters = pLocalCodecParameters;
          }

        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
          std::cout << "Audio Codec: " << pLocalCodecParameters->channels << " channels, sample rate "<< pLocalCodecParameters->channels << pLocalCodecParameters->sample_rate << std::endl;
        }

   }

   if (video_stream_index == -1) {
       return -1;
   }

   //分配编解码的上下文内存空间
   AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
   if (!pCodecContext)
   {
       return -1;
   }
   //填充参数
   if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
   {
       return -1;
   }
   if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0)
   {
       return -1;
   }
   //分配视频帧内存空间
   AVFrame *pFrame = av_frame_alloc();
   if (!pFrame)
   {
       return -1;
   }
   AVPacket *pPacket = av_packet_alloc();
   if (!pPacket)
   {
       return -1;
   }

   int response = 0;
   int how_many_packets_to_process = 10;

   while (av_read_frame(pFormatContext, pPacket) >= 0)
    {
      if (pPacket->stream_index == video_stream_index)
      {
        response = decode_packet(pPacket, pCodecContext, pFrame);
        if (response < 0)
          break;
        if (--how_many_packets_to_process <= 0) break;
      }
      av_packet_unref(pPacket);
    }


   avformat_close_input(&pFormatContext);
   av_packet_free(&pPacket);
   av_frame_free(&pFrame);
   avcodec_free_context(&pCodecContext);
}

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    QFont font;
    font.setFamily("Microsoft Yahei");
    font.setPixelSize(13);
    a.setFont(font);

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#else
    // QTextCodec *codec = QTextCodec::codecForName("utf-8");
    // QTextCodec::setCodecForLocale(codec);
#endif

    MainWindow w;
    w.setWindowTitle("ffmpeg_test");
    w.resize(1000, 800);
    w.show();
    return a.exec();
}


static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
  // Supply raw packet data as input to a decoder
  int response = avcodec_send_packet(pCodecContext, pPacket);

  if (response < 0) {
    return response;
  }

  while (response >= 0)
  {
      printf(
          "Frame %c (%d) pts %d dts %d key_frame %d [coded_picture_number %d, display_picture_number %d]",
          av_get_picture_type_char(pFrame->pict_type),
          pCodecContext->frame_number,
          pFrame->pts,
          pFrame->pkt_dts,
          pFrame->key_frame,
          pFrame->coded_picture_number,
          pFrame->display_picture_number
      );

    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      return response;
    }

    if (response >= 0) {

        printf(
            "Frame %c (%d) pts %d dts %d key_frame %d [coded_picture_number %d, display_picture_number %d]",
            av_get_picture_type_char(pFrame->pict_type),
            pCodecContext->frame_number,
            pFrame->pts,
            pFrame->pkt_dts,
            pFrame->key_frame,
            pFrame->coded_picture_number,
            pFrame->display_picture_number
        );

      char frame_filename[1024];
      snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
      if (pFrame->format != AV_PIX_FMT_YUV420P)
      {
        std::cout << "Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB"<< std::endl;
      }
      // save a grayscale frame into a .pgm file
      save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
    }
  }
  return 0;
}

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
