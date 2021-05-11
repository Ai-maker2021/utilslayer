#-------------------------------------------------
#
# Project created by QtCreator 2021-04-20T18:41:24
#
#-------------------------------------------------

#QT       -= gui

TARGET = utilslayer
TEMPLATE = lib

DEFINES += ARCFACIAL_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DESTDIR = $$_PRO_FILE_PWD_/build
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += \
        inc \
        camera \
        $$[QT_SYSROOT]/usr/include/drm \
        $$[QT_SYSROOT]/usr/include/rkaiq/algos \
        $$[QT_SYSROOT]/usr/include/rkaiq/common \
        $$[QT_SYSROOT]/usr/include/rkaiq/iq_parser \
        $$[QT_SYSROOT]/usr/include/rkaiq/uAPI \
        $$[QT_SYSROOT]/usr/include/rkaiq/xcore

SOURCES += \
        utils/utils.cpp \
        sound/play_wav.cpp \
        camera/camrgb_control.c \
        camera/camir_control.c \
        camera/aiq_control.cpp \
        drm/display.c \
        drm/rkdrm_display.c \
        drm/draw_rect.c \
        serialport/SerialPort.cpp \
        serialport/SerialPortBase.cpp \
        serialport/SerialPortInfo.cpp \
        serialport/SerialPortInfoBase.cpp \
        serialport/SerialPortInfoUnixBase.cpp \
        serialport/SerialPortUnixBase.cpp

HEADERS += \
        sound/play_wav.h \
        camera/aiq_control.h \
        camera/cam_control.h \
        drm/display.h \
        drm/rkdrm_display.h \
        drm/draw_rect.h \
        utilslayer.h

LIBS += \
    -lrkaiq \
    -lrkisp_api \
    -lrga \
    -ldrm \
    -lasound

unix {
    target.path = /usr/lib
    INSTALLS += target
}
