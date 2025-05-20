#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ffmpeg.h"

#include <QFileDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_ffmpeg = new aos::ffmpeg(ui->ffmpegWidget);
//    setCentralWidget(m_ffmpeg);

    QHBoxLayout* layout = new QHBoxLayout(ui->ffmpegWidget);
    layout->addWidget(m_ffmpeg);
    ui->ffmpegWidget->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{

}

void MainWindow::on_pushButton_2_clicked()
{
    //重新封装将文件从一种格式转换到另外一种格式
//    AVFormatContext *input_format_context = nullptr, *output_format_context = nullptr;
//    AVPacket packet;
//    std::string in_filename = "small_bunny_1080p_60fps.mp4";
//    std::string out_filename = "small_bunny_1080p_60fps.ts";
//    int *streams_list = nullptr;
//    int ret;
//    int stream_index = 0;
//    int number_of_streams = 0;
//     int fragmented_mp4_options = 0;

//    if ((ret = avformat_open_input(&input_format_context, in_filename.c_str(), nullptr, nullptr)) < 0)
//    {
//        fprintf(stderr, "Could not open input file '%s'", in_filename.c_str());
//        goto end;
//    }
//    if ((ret = avformat_find_stream_info(input_format_context, nullptr)) < 0)
//    {
//        fprintf(stderr, "Failed to retrieve input stream information");
//        goto end;
//    }
//    //申请转换文件容器的内存空间
//    avformat_alloc_output_context2(&output_format_context, nullptr, nullptr, out_filename.c_str());
//    if (!output_format_context)
//    {
//       fprintf(stderr, "Could not create output context\n");
//       ret = AVERROR_UNKNOWN;
//       goto end;
//    }

//    //视频、音频、字幕流需要存入数组中
//    number_of_streams = input_format_context->nb_streams;
//    streams_list = (int*)av_mallocz_array(number_of_streams, sizeof(*streams_list));
//    if (!streams_list) {
//        ret = AVERROR(ENOMEM);
//        goto end;
//    }
//    for (int i = 0; i < input_format_context->nb_streams; i++)
//    {
//      AVStream *out_stream;
//      AVStream *in_stream = input_format_context->streams[i];
//      AVCodecParameters *in_codecpar = in_stream->codecpar;
//      if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
//          in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
//          in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
//        streams_list[i] = -1;
//        continue;
//      }
//      streams_list[i] = stream_index++;
//      //遍历所有流，为所有流创建对应的输出流
//      out_stream = avformat_new_stream(output_format_context, nullptr);
//      if (!out_stream) {
//        fprintf(stderr, "Failed allocating output stream\n");
//        ret = AVERROR_UNKNOWN;
//        goto end;
//      }
//      ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
//      if (ret < 0) {
//        fprintf(stderr, "Failed to copy codec parameters\n");
//        goto end;
//      }
//    }
//    //创建一个输出文件
//    av_dump_format(output_format_context, 0, out_filename.c_str(), 1);
//    if (!(output_format_context->oformat->flags & AVFMT_NOFILE))
//    {
//        ret = avio_open(&output_format_context->pb, out_filename.c_str(), AVIO_FLAG_WRITE);
//        if (ret < 0) {
//          fprintf(stderr, "Could not open output file '%s'", out_filename.c_str());
//          goto end;
//        }
//    }
//      AVDictionary* opts = NULL;
//      if (fragmented_mp4_options) {
//        // https://developer.mozilla.org/en-US/docs/Web/API/Media_Source_Extensions_API/Transcoding_assets_for_MSE
//        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
//      }
//      ret = avformat_write_header(output_format_context, &opts);
//      if (ret < 0) {
//         fprintf(stderr, "Error occurred when opening output file\n");
//         goto end;
//      }
//      while (1) {
//        AVStream *in_stream, *out_stream;
//        ret = av_read_frame(input_format_context, &packet);
//        if (ret < 0)
//          break;
//        in_stream  = input_format_context->streams[packet.stream_index];
//        if (packet.stream_index >= number_of_streams || streams_list[packet.stream_index] < 0) {
//          av_packet_unref(&packet);
//          continue;
//        }
//        packet.stream_index = streams_list[packet.stream_index];
//        out_stream = output_format_context->streams[packet.stream_index];
//        /* copy packet */
//        //packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
//        //packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
//        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
//        // https://ffmpeg.org/doxygen/trunk/structAVPacket.html#ab5793d8195cf4789dfb3913b7a693903
//        packet.pos = -1;

//        //https://ffmpeg.org/doxygen/trunk/group__lavf__encoding.html#ga37352ed2c63493c38219d935e71db6c1
//        ret = av_interleaved_write_frame(output_format_context, &packet);
//        if (ret < 0) {
//          fprintf(stderr, "Error muxing packet\n");
//          break;
//        }
//        av_packet_unref(&packet);
//      }

//end:
//  avformat_close_input(&input_format_context);
//  /* close output */
//  if (output_format_context && !(output_format_context->oformat->flags & AVFMT_NOFILE))
//    avio_closep(&output_format_context->pb);
//  avformat_free_context(output_format_context);
//  av_freep(&streams_list);
//  if (ret < 0 && ret != AVERROR_EOF) {
//    fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
//  }

}

void MainWindow::on_btnSelect_clicked()
{
    QString fileName = QFileDialog::getOpenFileName();
    if (!fileName.isEmpty()) {
        ui->cboxUrl->addItem(fileName);
        ui->cboxUrl->lineEdit()->setText(fileName);
        if (ui->btnOpen->text() == "open") {
            on_btnOpen_clicked();
        }
    }
}


void MainWindow::on_btnOpen_clicked()
{

    if (ui->btnOpen->text() == "open") {
        ui->btnOpen->setText("close");
        QString url = ui->cboxUrl->currentText().trimmed();
        m_ffmpeg->setUrl(url);
        m_ffmpeg->open();
    } else {
        ui->btnOpen->setText("open");
        m_ffmpeg->close();
    }

}
