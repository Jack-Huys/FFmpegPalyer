#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ffmpegCommon.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace aos{
    class ffmpeg;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_btnSelect_clicked();
    void on_btnOpen_clicked();

private:
    Ui::MainWindow *ui;
    aos::ffmpeg* m_ffmpeg;
};
#endif // MAINWINDOW_H
