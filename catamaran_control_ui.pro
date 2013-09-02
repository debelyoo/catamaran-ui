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
    sensorConfig.cpp\
    util/byteArrayConverter.cpp\
    util/fileHelper.cpp\
    util/coordinateHelper.cpp\
    model/sensor.cpp\
    gui/mainwindow.cpp\
    gui/inRowCheckBox.cpp\
    gui/inRowComboBox.cpp\
    gui/inRowLineEdit.cpp\
    communication/server.cpp\
    communication/messageConsumer.cpp\
    communication/messagePublisher.cpp

HEADERS  += sensorConfig.h\
    util/byteArrayConverter.h\
    util/fileHelper.h\
    util/coordinateHelper.h\
    model/sensor.h\
    gui/mainwindow.h\
    gui/inRowCheckBox.h\
    gui/inRowComboBox.h\
    gui/inRowLineEdit.h\
    communication/server.h\
    communication/messageConsumer.h\
    communication/messagePublisher.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc

OTHER_FILES +=
