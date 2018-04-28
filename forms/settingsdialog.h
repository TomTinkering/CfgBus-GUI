/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QSettings>


QT_BEGIN_NAMESPACE

namespace Ui {
class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

struct Settings {
    //modbus (default)settings
    uint32_t maxMonitorLines = 50;
    QString  strMaxMonitorLines = "50";
    uint32_t slave = 0;
    QString  strSlave = "0";
    uint32_t respTimeout = 0;
    QString  strRespTimeout = "0";

    //serial port (default)settings
    QString portName = "COM1";
    QString portLocation = "\\\\.\\COM1";
    qint32 baudRate = 115200;
    QString strBaudRate = "115200";
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QString strDataBits = "8";
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QString strParity = "None";
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QString strStopBits = "1";
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    QString strFlowControl = "None";
};

class SettingsFileHandler: public QSettings
{
public:
    explicit SettingsFileHandler(QWidget *parent = nullptr):
        QSettings("qCfgMaster.ini",QSettings::IniFormat,parent)
    {
    }

    Settings loadSettings();
    void saveSettings(const Settings& settings);

private:

};


class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    const Settings& settings() const
    {
        return m_settings;
    }

    void loadSettings()
    {
        m_settings = m_handler->loadSettings();
    }

    void saveSettings()
    {
        m_handler->saveSettings(m_settings);
    }

private slots:
    void showPortInfo(int idx);
    void accepted();
    void rejected();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

private:
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

private:
    Settings m_settings;
    SettingsFileHandler* m_handler;
    Ui::SettingsDialog *m_ui = nullptr;
    QIntValidator *m_intValidator = nullptr;
};

#endif // SETTINGSDIALOG_H
