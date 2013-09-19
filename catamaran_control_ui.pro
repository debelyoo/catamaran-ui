#-------------------------------------------------
#
# Project created by QtCreator 2013-08-19T10:22:56
#
#-------------------------------------------------

QT += core gui network sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = catamaran_control_ui
CONFIG +=  static
TEMPLATE = app


SOURCES += main.cpp\
    sensorConfig.cpp\
    util/byteArrayConverter.cpp\
    util/fileHelper.cpp\
    util/coordinateHelper.cpp\
    util/databaseManager.cpp \
    gui/mainwindow.cpp\
    gui/inRowCheckBox.cpp\
    gui/inRowComboBox.cpp\
    gui/inRowLineEdit.cpp\
    gui/qcustomplot.cpp \
    gui/dataPlot.cpp \
    gui/mouseClickHandler.cpp \
    gui/model/sensorinputsmodel.cpp \
    gui/model/sensorinputitem.cpp \
    gui/model/registeredsensoritem.cpp \
    model/sensorType.cpp \
    model/sensor.cpp\
    communication/dataObject.cpp \
    communication/server.cpp\
    communication/messageConsumer.cpp\
    communication/messagePublisher.cpp \
    transformation/transformationbaseclass.cpp \
    gui/model/registeredsensorsmodel.cpp \
    gui/delegate/registeredSensorsDelegate.cpp \
    gui/sensortransformationconfig.cpp \
    transformation/transformationmanager.cpp

HEADERS  += sensorConfig.h\
    util/byteArrayConverter.h\
    util/fileHelper.h\
    util/coordinateHelper.h\
    util/definitions.h\
    util/databaseManager.h \
    util/timeHelper.h \
    model/sensor.h\
    model/sensorType.h \
    communication/server.h\
    communication/messageConsumer.h\
    communication/messagePublisher.h \
    communication/dataObject.h \
    gui/mainwindow.h\
    gui/inRowCheckBox.h\
    gui/inRowComboBox.h\
    gui/inRowLineEdit.h\
    gui/qcustomplot.h \
    gui/dataPlot.h \
    gui/mouseClickHandler.h \
    gui/model/sensorinputsmodel.h \
    gui/model/sensorinputitem.h \
    gui/model/registeredsensoritem.h \
    communication/idatamessagereceiver.h \
    transformation/transformationbaseclass.h \
    gui/model/registeredsensorsmodel.h \
    gui/delegate/registeredSensorsDelegate.h \
    gui/sensortransformationconfig.h \
    transformation/transformationmanager.h



FORMS    += mainwindow.ui \
    gui/sensortransformationconfig.ui

RESOURCES += \
    images.qrc

OTHER_FILES +=
