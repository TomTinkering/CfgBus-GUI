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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_handler(new SettingsFileHandler(parent)),
    m_ui(new Ui::SettingsDialog),
    m_intValidator(new QIntValidator(0, 4000000, this))
{
    m_ui->setupUi(this);

    m_ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(m_ui->buttonBox,SIGNAL(accepted()),this,SLOT(accepted()));
    connect(m_ui->buttonBox,SIGNAL(rejected()),this,SLOT(rejected()));
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::showPortInfo);
    connect(m_ui->baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomBaudRatePolicy);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomDevicePathPolicy);

    loadSettings();

    fillPortsInfo();
    fillPortsParameters();
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    const QStringList list = m_ui->serialPortInfoListBox->itemData(idx).toStringList();
    m_ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    m_ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    m_ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    m_ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    m_ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    m_ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingsDialog::accepted()
{
    updateSettings();
    saveSettings();
    hide();
    QDialog::accept();
}

void SettingsDialog::rejected()
{
    hide();
    QDialog::reject();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !m_ui->baudRateBox->itemData(idx).isValid();
    m_ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        m_ui->baudRateBox->clearEditText();
        QLineEdit *edit = m_ui->baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !m_ui->serialPortInfoListBox->itemData(idx).isValid();
    m_ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        m_ui->serialPortInfoListBox->clearEditText();
}

void SettingsDialog::fillPortsParameters()
{
    m_ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    m_ui->baudRateBox->addItem(tr("Custom"));

    int res = m_ui->baudRateBox->findText(m_settings.strBaudRate);
    if (res == -1)
        m_ui->baudRateBox->setCurrentIndex(3);
    else
        m_ui->baudRateBox->setCurrentIndex(res);


    m_ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);

    res = m_ui->dataBitsBox->findData(m_settings.dataBits);
    if (res == -1)
        m_ui->dataBitsBox->setCurrentIndex(3);
    else
        m_ui->dataBitsBox->setCurrentIndex(res);

    m_ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    m_ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    res = m_ui->parityBox->findData(m_settings.parity);
    if (res == -1)
        m_ui->parityBox->setCurrentIndex(0);
    else
        m_ui->parityBox->setCurrentIndex(res);


    m_ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    m_ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    m_ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    res = m_ui->stopBitsBox->findData(m_settings.stopBits);
    if (res == -1)
        m_ui->stopBitsBox->setCurrentIndex(0);
    else
        m_ui->stopBitsBox->setCurrentIndex(res);


    m_ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    m_ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    m_ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);

    res = m_ui->flowControlBox->findData(m_settings.flowControl);
    if (res == -1)
        m_ui->stopBitsBox->setCurrentIndex(0);
    else
        m_ui->stopBitsBox->setCurrentIndex(res);

    m_ui->sbMaxNoOfRawDataLines->setValue(m_settings.maxMonitorLines);
    m_ui->sbResponseTimeout->setValue(m_settings.respTimeout);
    m_ui->sbBaseAddr->setValue(m_settings.slave);

    res = m_ui->serialPortInfoListBox->findText(m_settings.portName);
    if (res == -1)
        m_ui->serialPortInfoListBox->setCurrentIndex(0);
    else
        m_ui->serialPortInfoListBox->setCurrentIndex(res);
}

void SettingsDialog::fillPortsInfo()
{
    m_ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        m_ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    m_ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void SettingsDialog::updateSettings()
{
   m_settings.portName = m_ui->serialPortInfoListBox->currentText();

    const QStringList list =  m_ui->serialPortInfoListBox->currentData().toStringList();
   m_settings.portLocation = (list.count() > 4) ? list.at(4) : tr(blankString);

    if (m_ui->baudRateBox->currentIndex() == 4) {
       m_settings.baudRate = m_ui->baudRateBox->currentText().toInt();
    } else {
       m_settings.baudRate = static_cast<QSerialPort::BaudRate>(
                    m_ui->baudRateBox->itemData(m_ui->baudRateBox->currentIndex()).toInt());
    }

   m_settings.strBaudRate = QString::number(m_settings.baudRate);

   m_settings.dataBits = static_cast<QSerialPort::DataBits>(
                m_ui->dataBitsBox->itemData(m_ui->dataBitsBox->currentIndex()).toInt());
   m_settings.strDataBits = m_ui->dataBitsBox->currentText();

   m_settings.parity = static_cast<QSerialPort::Parity>(
                m_ui->parityBox->itemData(m_ui->parityBox->currentIndex()).toInt());
   m_settings.strParity = m_ui->parityBox->currentText();

   m_settings.stopBits = static_cast<QSerialPort::StopBits>(
                m_ui->stopBitsBox->itemData(m_ui->stopBitsBox->currentIndex()).toInt());
   m_settings.strStopBits = m_ui->stopBitsBox->currentText();

   m_settings.flowControl = static_cast<QSerialPort::FlowControl>(
                m_ui->flowControlBox->itemData(m_ui->flowControlBox->currentIndex()).toInt());
   m_settings.strFlowControl = m_ui->flowControlBox->currentText();

   m_settings.strMaxMonitorLines = m_ui->sbMaxNoOfRawDataLines->cleanText();
   m_settings.strRespTimeout = m_ui->sbResponseTimeout->cleanText();
   m_settings.strSlave = m_ui->sbBaseAddr->cleanText();

   m_settings.maxMonitorLines = m_ui->sbMaxNoOfRawDataLines->value();
   m_settings.respTimeout = m_ui->sbResponseTimeout->value();
   m_settings.slave = m_ui->sbBaseAddr->value();
}


Settings SettingsFileHandler::loadSettings()
{
    Settings result;

    if (!this->value("portName").isNull())
        result.portName = this->value("portName").toString();

    if (!this->value("portLocation").isNull())
        result.portLocation = this->value("portLocation").toString();

    if (!this->value("strBaudRate").isNull())
        result.strBaudRate = this->value("strBaudRate").toString();

    if (!this->value("strDataBits").isNull())
        result.strDataBits = this->value("strDataBits").toString();

    if (!this->value("strParity").isNull())
        result.strParity = this->value("strParity").toString();

    if (!this->value("strStopBits").isNull())
        result.strStopBits = this->value("strStopBits").toString();

    if (!this->value("strFlowControl").isNull())
        result.strFlowControl = this->value("strFlowControl").toString();

    if (!this->value("strMaxMonitorLines").isNull())
        result.strMaxMonitorLines = this->value("strMaxMonitorLines").toString();

    if (!this->value("strSlave").isNull())
        result.strSlave = this->value("strSlave").toString();

    if (!this->value("strRespTimeout").isNull())
       result.strRespTimeout = this->value("strRespTimeout").toString();

    QVariant val = this->value("baudRate");
    if (!val.isNull())
        result.baudRate = val.value<uint32_t>();

    val = this->value("dataBits");
    if (!val.isNull())
        result.dataBits = val.value<QSerialPort::DataBits>();

    val = this->value("parity");
    if (!val.isNull())
        result.parity = val.value<QSerialPort::Parity>();

    val = this->value("flowControl");
    if (!val.isNull())
        result.flowControl = val.value<QSerialPort::FlowControl>();

    val = this->value("stopBits");
    if (!val.isNull())
        result.stopBits = val.value<QSerialPort::StopBits>();

    val = this->value("maxMonitorLines");
    if (!val.isNull())
        result.maxMonitorLines = val.value<uint32_t>();

    val = this->value("slave");
    if (!val.isNull())
        result.slave = val.value<uint32_t>();

    val = this->value("respTimeout");
    if (!val.isNull())
        result.respTimeout = val.value<uint32_t>();

    return result;
}

void  SettingsFileHandler::saveSettings(const Settings& settings)
{
    this->setValue("maxMonitorLines",settings.maxMonitorLines);
    this->setValue("strMaxMonitorLines",settings.strMaxMonitorLines);
    this->setValue("slave",settings.slave);
    this->setValue("strSlave",settings.strSlave);
    this->setValue("respTimeout",settings.respTimeout);
    this->setValue("strRespTimeout",settings.strRespTimeout);
    this->setValue("portName",settings.portName);
    this->setValue("portLocation",settings.portLocation);
    this->setValue("baudRate",settings.baudRate);
    this->setValue("strBaudRate",settings.strBaudRate);
    this->setValue("dataBits",settings.dataBits);
    this->setValue("strDataBits",settings.strDataBits);
    this->setValue("parity",settings.parity);
    this->setValue("strParity",settings.strParity);
    this->setValue("stopBits",settings.stopBits);
    this->setValue("strStopBits",settings.strStopBits);
    this->setValue("flowControl",settings.flowControl);
    this->setValue("strFlowControl",settings.strFlowControl);
}




