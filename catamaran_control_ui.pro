#-------------------------------------------------
#
# Project created by QtCreator 2013-08-19T10:22:56
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = catamaran_control_ui
CONFIG +=  static
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    messageConsumer.cpp \
    byteArrayConverter.cpp \
    messagePublisher.cpp \
    sensorConfig.cpp \
    sensor.cpp \
    coordinateHelper.cpp \
    inRowLineEdit.cpp \
    inRowCheckBox.cpp \
    inRowComboBox.cpp \
    fileHelper.cpp

HEADERS  += mainwindow.h \
    server.h \
    messageConsumer.h \
    byteArrayConverter.h \
    messagePublisher.h \
    sensorConfig.h \
    sensor.h \
    coordinateHelper.h \
    inRowLineEdit.h \
    inRowComboBox.h \
    inRowCheckBox.h \
    fileHelper.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc
