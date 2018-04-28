#-------------------------------------------------
#
# Project created by QtCreator 2010-11-24T09:57:26
#
#-------------------------------------------------

QT       += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qCfgMaster
TEMPLATE = app

SOURCES +=  src/main.cpp \
            src/mainwindow.cpp \
            3rdparty/libmodbus/modbus.c \
            forms/about.cpp \
            forms/MyInfoBar.cpp \
            src/eutils.cpp \
            forms/busmonitor.cpp \
            3rdparty/libmodbus/modbus-data.c \
            3rdparty/libmodbus/modbus-tcp.c \
            3rdparty/libmodbus/modbus-rtu.c \
            3rdparty/QsLog/QsLogDest.cpp \
            3rdparty/QsLog/QsLog.cpp \
            3rdparty/QsLog/QsLogDestConsole.cpp \
            3rdparty/QsLog/QsLogDestFile.cpp \            
            src/cfgbusentry.cpp \
            src/cfgbusmodel.cpp \
            forms/settingsdialog.cpp \
            src/cfgbusmaster.cpp

HEADERS  += 3rdparty/libmodbus/modbus.h \
            forms/about.h \
            forms/busmonitor.h \
            forms/MyInfoBar.h \
            3rdparty/QsLog/QsLog.h \
            3rdparty/QsLog/QsLogDest.h \
            3rdparty/QsLog/QsLogDestConsole.h \
            3rdparty/QsLog/QsLogLevel.h \
            3rdparty/QsLog/QsLogDisableForThisFile.h \
            3rdparty/QsLog/QsLogDestFile.h \
            inc/mainwindow.h \
            inc/cfgbus.h \
            inc/cfgbusentry.h \
            inc/cfgbusmodel.h \
            forms/settingsdialog.h \
            inc/cfgbusmaster.h \
    inc/eutils.h

INCLUDEPATH += 3rdparty/libmodbus \
               3rdparty/QsLog \
               inc \
               forms


TRANSLATIONS += translations/$$TARGET"_zh_CN.ts"
TRANSLATIONS += translations/$$TARGET"_zh_TW.ts"

unix:SOURCES +=

unix:DEFINES += _TTY_POSIX_

win32:SOURCES +=

win32:DEFINES += _TTY_WIN_  WINVER=0x0501

win32:LIBS += -lsetupapi -lwsock32 -lws2_32

QMAKE_CXXFLAGS += -std=gnu++11

DEFINES += QS_LOG_LINE_NUMBERS     # automatically writes the file and line for each log message
#DEFINES += QS_LOG_DISABLE         # logging code is replaced with a no-op
#DEFINES += QS_LOG_SEPARATE_THREAD # messages are queued and written from a separate thread
#DEFINES += LIB_MODBUS_DEBUG_OUTPUT # enable debug output from libmodbus

FORMS    += forms/mainwindow.ui \
            forms/about.ui \
            forms/busmonitor.ui \
    forms/settingsdialog.ui

RESOURCES += icons/icons.qrc


















