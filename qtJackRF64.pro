#-------------------------------------------------
#
# Project created by QtCreator 2017-11-15T21:00:04
#
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtJackRF64
TEMPLATE = app

#INCLUDEPATH += `pkg-config --cflags jack`
LIBS += `pkg-config --libs jack` -lrt -lm -lpthread -lsndfile

SOURCES += main.cpp\
        window.cpp \
    jackclient.cpp \
    netclient.cpp \
    receiver.cpp \
    sender.cpp \
    soundfile.cpp \
    clibrary/byte-order.c

HEADERS  += window.h \
    jackclient.h \
    netclient.h \
    receiver.h \
    sender.h \
    soundfile.h \
    clibrary/byte-order.h \
    clibrary/float.h \
    clibrary/int.h \
    clibrary/failure.h \
    clibrary/print.h

FORMS    += window.ui
