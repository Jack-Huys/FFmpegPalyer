QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS


DEFINES += ffmpeg4
contains(DEFINES, ffmpeg4) {
path_main = ffmpeg4
} else {
path_main = ffmpeg3
}

contains(QT_ARCH, x86_64) {
path_lib = libwin64
} else {
path_lib = libwin32
}

#包含头文件
INCLUDEPATH += $$PWD/3rd/ffmpeg/$$path_main/include
#链接库文件
LIBS += -L$$PWD/3rd/ffmpeg/$$path_main/$$path_lib/ -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil


SOURCES += \
    ffmpeg.cpp \
    ffmpegthread.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ffmpeg.h \
    ffmpegCommon.h \
    ffmpegthread.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
