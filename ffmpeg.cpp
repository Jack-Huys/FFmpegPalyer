#include "ffmpeg.h"
#include "ffmpegthread.h"

namespace aos {

ffmpeg::ffmpeg(QWidget *parent) : QWidget(parent)
{
    m_thread = new aos::ffmpegThread(this);
    connect(m_thread, SIGNAL(receiveImage(QImage)), this, SLOT(updateImage(QImage)));
    //connect(m_thread, &aos::ffmpegThread::receiveImage, this, &aos::ffmpeg::updateImage);
    m_image = QImage();
}

ffmpeg::~ffmpeg()
{
    close();
}

void ffmpeg::paintEvent(QPaintEvent *)
{
    if (m_image.isNull()) {
        return;
    }

    QPainter painter(this);
#if 0
    //image = image.scaled(this->size(), Qt::KeepAspectRatio);
    //按照比例自动居中绘制
    int pixX = rect().center().x() - m_image.width() / 2;
    int pixY = rect().center().y() - m_image.height() / 2;
    QPoint point(pixX, pixY);
    painter.drawImage(point, m_image);
#else
    painter.drawImage(this->rect(), m_image);
//   painter.setRenderHint(QPainter::Antialiasing);

//   // 定义你想要绘制图片的矩形区域
//   QRect targetRect(0, 0, width(), height());

//   // 计算保持宽高比的绘制区域
//   int newWidth = targetRect.width();
//   int newHeight = newWidth * m_image.height() / m_image.width();
//   if (newHeight > targetRect.height()) {
//       newHeight = targetRect.height();
//       newWidth = newHeight * m_image.width() / m_image.height();
//   }

//   // 计算绘制区域的左上角位置，保持居中
//   int x = (targetRect.width() - newWidth) / 2;
//   int y = (targetRect.height() - newHeight) / 2;

//   // 绘制图片到目标矩形区域，保持宽高比
//   painter.drawImage(QRect(x, y, newWidth, newHeight), m_image);
#endif


}

void ffmpeg::updateImage(const QImage &image)
{
    this->m_image = image;
    this->update();
}

void ffmpeg::setUrl(const QString &url)
{
    m_thread->setUrl(url);
}

void ffmpeg::open()
{
    clear();

    m_thread->play();
    m_thread->start();
}

void ffmpeg::pause()
{
    m_thread->pause();
}

void ffmpeg::next()
{
    m_thread->next();
}

void ffmpeg::close()
{
    if (m_thread->isRunning()) {
        m_thread->stop();
        m_thread->quit();
        m_thread->wait(500);
    }

    QTimer::singleShot(1, this, SLOT(clear()));
}

void ffmpeg::clear()
{
    m_image = QImage();
    update();
}

}
