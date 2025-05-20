#ifndef FFMPEG_H
#define FFMPEG_H

#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#include "ffmpegthread.h"

namespace aos {

class ffmpeg : public QWidget
{
    Q_OBJECT

public:
    explicit ffmpeg(QWidget *parent = nullptr);
    ~ffmpeg();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    //接收图像并绘制
    void updateImage(const QImage &image);

public slots:

    void setUrl(const QString &url);
    void open();
    void pause();
    void next();
    void close();
    void clear();

private:
    aos::ffmpegThread *m_thread;
    QImage m_image;

};

}


#endif // FFMPEG_H
