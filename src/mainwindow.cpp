#include <QtDebug>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QTranslator>
#include <QString>
#include <Qtimer>


#include "QsLog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "eutils.h"
#include "chartwindow.h"

MainWindow *mainWin;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_model(new CfgBusModel(this))
{
    ui->setupUi(this);

    //UI - dialogs
    m_dlgAbout = new About();
    connect(ui->actionAbout,SIGNAL(triggered()),m_dlgAbout,SLOT(show()));
    m_dlgSettings = new SettingsDialog(this);
    connect(ui->actionSettings,SIGNAL(triggered()),this,SLOT(showSettings()));
    m_busMonitor = new BusMonitor(this);//, m_model->rawModel);
    connect(ui->actionBus_Monitor,SIGNAL(triggered()),this,SLOT(showBusMonitor()));
    m_chartWindow = new ChartWindow(this);

    m_busMonitor->setWindowFlags(Qt::Widget);
    ui->busMonLayout->addWidget(m_busMonitor);

    m_chartWindow->setWindowFlags(Qt::Widget);
    ui->chartLayout->addWidget(m_chartWindow);

    //UI - connections
    connect(ui->spInterval,SIGNAL(valueChanged(int)),this,SLOT(changedScanRate(int)));
    connect(ui->actionClear,SIGNAL(triggered()),this,SLOT(clearItems()));
    connect(ui->actionRead_Write,SIGNAL(triggered()),this,SLOT(request()));
    connect(ui->actionGetEntryList,SIGNAL(triggered()),this,SLOT(getEntryList()));
    connect(ui->actionScan,SIGNAL(toggled(bool)),this,SLOT(scan(bool)));
    connect(ui->actionConnect,SIGNAL(toggled(bool)),this,SLOT(changedConnect(bool)));
    connect(ui->actionReset_Counters,SIGNAL(triggered()),this,SIGNAL(resetCounters()));
    connect(ui->actionOpenLogFile,SIGNAL(triggered()),this,SLOT(openLogFile()));
    connect(ui->actionModbus_Manual,SIGNAL(triggered()),this,SLOT(openModbusManual()));

    //UI - status
    m_statusInd = new QLabel;
    m_statusInd->setFixedSize( 16, 16 );
    m_statusText = new QLabel;
    m_statusPackets = new QLabel(tr("Packets : ") + "0");
    m_statusPackets->setStyleSheet("QLabel {color:blue;}");
    m_statusErrors = new QLabel(tr("Errors : ") + "0");
    m_statusErrors->setStyleSheet("QLabel {color:red;}");

    ui->statusBar->addWidget(m_statusInd);
    ui->statusBar->addWidget(m_statusText, 10);
    ui->statusBar->addWidget(m_statusPackets, 10);
    ui->statusBar->addWidget(m_statusErrors, 10);
    m_statusInd->setPixmap(QPixmap(":/img/ballorange-16.png"));

    //Init models
    ui->tblRegisters->setModel(m_model);

    //Update UI
    ui->actionRead_Write->setEnabled(false);
    ui->actionScan->setEnabled(false);
    ui->actionGetEntryList->setEnabled(false);
    updateStatusBar();

    //Update TableView
    ui->tblRegisters->setColumnWidth(Cfg::IDColumn,30);
    ui->tblRegisters->setColumnWidth(Cfg::AddrColumn,50);
    ui->tblRegisters->setColumnWidth(Cfg::RWColumn,40);
    ui->tblRegisters->setColumnWidth(Cfg::SizeColumn,40);
    ui->tblRegisters->setColumnWidth(Cfg::TypeColumn,60);
    ui->tblRegisters->setColumnWidth(Cfg::NameColumn,125);
    ui->tblRegisters->setColumnWidth(Cfg::ValueColumn,125);
    ui->tblRegisters->setFixedWidth(500);

    ui->tblRegisters->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tblRegisters->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //setup timer
    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(request()));

    //setup charts
//    m_chart = new ChartWidget(this);
//    ui->gridLayout->addWidget(m_chart,1,1);

    QLOG_INFO()<<  "Start Program" ;

}

MainWindow::~MainWindow()
{

    delete ui;

    QLOG_INFO()<<  "Stop Program" ;

}

void MainWindow::showSettings()
{
    //Show General Settings Dialog
    auto res = m_dlgSettings->exec();
    if (res==QDialog::Accepted) {
        QLOG_INFO()<<  "Settings changes accepted ";
        //m_model->rawModel->setMaxNoOfLines(m_dlgSettings->settings().maxMonitorLines);
        //m_model->setTimeOut(m_dlgSettings->settings().respTimeout);
        m_model->setTimeout(m_dlgSettings->settings().respTimeout);
        m_dlgSettings->saveSettings();
    }
    else
        QLOG_INFO()<<  "Settings changes rejected ";
}

void MainWindow::showBusMonitor()
{
    //Show Bus Monitor
    //m_model->rawModel->setMaxNoOfLines(m_dlgSettings->settings().maxMonitorLines);
    m_busMonitor->move(this->x() + this->width() + 20, this->y());
    m_busMonitor->show();
}


void MainWindow::changedScanRate(int value)
{

    //Enable-Disable Time Interval

    QLOG_INFO()<<  "ScanRate changed. Value = " << value;

    //m_model->setScanRate(value);

}

void MainWindow::changedConnect(bool value)
{

    //Connect - Disconnect

    if (value) { //Connected
        modbusConnect(true);
        QLOG_INFO()<<  "Connected ";
    }
    else { //Disconnected
        modbusConnect(false);
        QLOG_INFO()<<  "Disconnected ";
    }

    m_model->resetCounters();
    refreshView();

}

void MainWindow::changedSlaveIP()
{

    // NOT USED

}

void MainWindow::openLogFile()
{

    //Open log file
    QString arg;
    QLOG_INFO()<<  "Open log file";

    arg = "file:///" + QCoreApplication::applicationDirPath() + "/QModMaster.log";
    QDesktopServices::openUrl(QUrl(arg));


}

void MainWindow::openModbusManual()
{

    //Open Modbus Manual
    QString arg;
    QLOG_INFO()<<  "Open Modbus Manual";

    arg = "file:///" + QCoreApplication::applicationDirPath() + "/ManModbus/index.html";
    QDesktopServices::openUrl(QUrl(arg));


}

void MainWindow::updateStatusBar()
{

    //Update status bar

    QString msg;

    msg = "Slave [" + m_dlgSettings->settings().strSlave + "]: ";
    msg += m_dlgSettings->settings().portName + " | ";
    msg += m_dlgSettings->settings().strBaudRate + ",";
    msg += m_dlgSettings->settings().strDataBits + ",";
    msg += m_dlgSettings->settings().strStopBits + ",";
    msg += m_dlgSettings->settings().strParity + "   ";

    m_statusText->clear();
    m_statusText->setText(msg);

    //Connection is valid
    if (m_model->isConnected()) {
        m_statusInd->setPixmap(QPixmap(":/icons/bullet-green-16.png"));
    }
    else {
        m_statusInd->setPixmap(QPixmap(":/icons/bullet-red-16.png"));
    }

}


void MainWindow::clearItems()
{

    //Clear items from registers model

    QLOG_INFO()<<  "clearItems" ;

    m_model->clearEntries();
    ui->actionGetEntryList->setEnabled(m_model->isConnected());

}


void MainWindow::getEntryList()
{
    //Clear items from registers model
    QLOG_INFO()<<  "getEntryList" ;

    if(!m_model->isConnected())
        return;

    if (!m_model->retrieveEntryList()) {
       mainWin->showUpInfoBar(tr("Request failed\nGet Configbus Slave Entry List."), MyInfoBar::Error);
       QLOG_WARN()<<  "Request failed.";
       return;
    }
    else {
       mainWin->hideInfoBar();
       ui->actionGetEntryList->setEnabled(false);
    }

    MainWindow::request();

}


void MainWindow::request()
{
    //Clear items from registers model
    QLOG_INFO()<<  "updateList" ;

    if(!m_model->isConnected())
        return;

    if(m_model->getNrEntries() == 0)
        return;

    for(int i = 0; i<m_model->getNrEntries(); i++)
    {
        if(!m_model->updateEntry(i))
        {
            QString msg = "Request failed\nUpdate Entries, on entry: " + QString::number(i);
            mainWin->showUpInfoBar(tr(msg.toStdString().c_str()), MyInfoBar::Error);
            QLOG_WARN()<<  "Request failed.";
            return;
        }
    }

    mainWin->hideInfoBar();
}

void MainWindow::scan(bool value)
{
    //Start-Stop poll timer
    QLOG_INFO()<<  "Scan time = " << value;
    if (value){
        m_scanRate = ui->spInterval->value();
        m_timer->start(m_scanRate);
    }
    else
    {
        m_timer->stop();
    }

    ui->cmbBase->setEnabled(!value);
    ui->spInterval->setEnabled(!value);
    ui->actionRead_Write->setEnabled(!value);

}

void MainWindow::modbusConnect(bool connect)
 {

    //Modbus connect - RTU/TCP
    QLOG_INFO()<<  "Modbus Connect. Value = " << connect;

    m_model->setSlave(m_dlgSettings->settings().slave);

    if (connect) {
        m_model->modbusConnect( m_dlgSettings->settings().portLocation.toStdString(),
                                m_dlgSettings->settings().baudRate,
                                m_dlgSettings->settings().strParity.toStdString().c_str()[0],
                                m_dlgSettings->settings().dataBits,
                                m_dlgSettings->settings().stopBits,
                                m_dlgSettings->settings().flowControl,
                                m_dlgSettings->settings().respTimeout
                                );
    }
    else { //Disconnect
        m_model->modbusDisconnect();
        ui->actionScan->setChecked(false);
    }

    updateStatusBar();

    //Update UI
    ui->actionConnect->setChecked(m_model->isConnected());
    ui->actionRead_Write->setEnabled(m_model->isConnected());
    ui->actionScan->setEnabled(m_model->isConnected());
    ui->actionGetEntryList->setEnabled(m_model->isConnected());

 }

 void MainWindow::refreshView()
 {

     QLOG_INFO()<<  "Packets sent / received = " << m_model->getNrPackets() << ", errors = " << m_model->getNrErrors();
     //ui->tblRegisters->resizeColumnsToContents();

     m_statusPackets->setText(tr("Packets : ") + QString("%1").arg(m_model->getNrPackets()));
     m_statusErrors->setText(tr("Errors : ") + QString("%1").arg(m_model->getNrErrors()));

 }

void MainWindow::showUpInfoBar(QString message, MyInfoBar::InfoType type)
{
    ui->infobar->show(message, type);
}

void MainWindow::hideInfoBar()
{
    ui->infobar->hide();
}

void MainWindow::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::changeLanguage()
{
    extern QTranslator *Translator;
    QCoreApplication::removeTranslator(Translator);
    Translator->load(":/translations/" + QCoreApplication::applicationName() + sender()->objectName().right(6));
    QCoreApplication::installTranslator(Translator);
}

