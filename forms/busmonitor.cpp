#include <QtDebug>
#include <QsLog.h>
#include <QtDebug>
#include <QFile>
#include <QFileDialog>
#include <QCloseEvent>
#include <QShowEvent>
#include <QStandardItemModel>

#include "eutils.h"
#include "busmonitor.h"
#include "ui_busmonitor.h"

BusMonitor* m_instance;

BusMonitor::BusMonitor(QWidget *parent) : //, RawDataModel *rawDataModel) :
    QMainWindow(parent),
    ui(new Ui::BusMonitor),
    m_rawModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_instance = this;

    ui->lstRawData->setModel(m_rawModel);

    //Setup Toolbar
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(save()));
    connect(ui->actionClear,SIGNAL(triggered()),this,SLOT(clear()));
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(exit()));
    connect(ui->lstRawData,SIGNAL(activated(QModelIndex)),this,SLOT(selectedRow(QModelIndex)));
    connect(ui->lstRawData,SIGNAL(clicked(QModelIndex)),this,SLOT(selectedRow(QModelIndex)));

}

BusMonitor::~BusMonitor()
{
    delete ui;
}

void BusMonitor::save()
{

    qDebug()<<  "BusMonitor : save" ;

    //Select file
    QString fileName = QFileDialog::getSaveFileName(NULL,"Save File As...",
                                                    QDir::homePath(),"Text (*.txt)",0,
                                                    QFileDialog::DontConfirmOverwrite);

    //Open File
    if (fileName.isEmpty())
        return;
    //continue only if a file name exists
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) return;

    //Text Stream
    QTextStream ts(&file);

    for(int row=0; row<m_rawModel->rowCount(); row++)
    {
        QModelIndex index = m_rawModel->index(row,0);
        ts << m_rawModel->data(index).toString() << endl;
    }

    //Close File
    file.close();

}

void BusMonitor::clear()
{

    qDebug()<<  "BusMonitor : clear" ;

    m_rawModel->removeRows(0,m_rawModel->rowCount());
    ui->txtPDU->clear();

}

void BusMonitor::exit()
{

   qDebug()<<  "BusMonitor : exit" ;

   this->close();

}

void BusMonitor::closeEvent(QCloseEvent *event)
{

    //m_rawDataModel->enableAddLines(false);
    clear();
    event->accept();

}

void BusMonitor::showEvent(QShowEvent *event)
{

    //m_rawDataModel->enableAddLines(true);
    event->accept();

}

void BusMonitor::selectedRow(const QModelIndex & selected)
{
    if (selected.data().canConvert(QMetaType::QString)) {
            QString val = selected.data().value<QString>();
            qDebug()<<  "BusMonitor : selectedRow - " << val;
            parseMsg(val);
    }
}

static QString _toIntAndHexString(QString hi, QString lo)
{
    bool ok;
    auto hiInt = hi.toUInt(&ok,16);

    if(!ok)
        return 0;

    auto loInt = lo.toUInt(&ok,16);

    if(!ok)
        return 0;

    return QString::number((hiInt << 8) + loInt) + " [0x" + hi + lo + "]";
}

void BusMonitor::parseMsg(QString msg)
{

    QStringList pdu = msg.split(QRegExp("\\s+"));
    if(pdu[1] == "Tx")
        parseTxPDU(pdu);
    else if(pdu[1] == "Rx")
        parseRxPDU(pdu);
    else if(pdu[1] == "Sys")
        parseSysMsg(pdu[2]);
    else
        ui->txtPDU->setPlainText("Type : Unknown Message");

    int ofs = (pdu[pdu.length() - 1] == "") ? 1 : 0;
    auto hi = pdu[pdu.length() - 2 - ofs];
    auto lo = pdu[pdu.length() - 1 - ofs];

    ui->txtPDU->appendPlainText("CRC : " +  _toIntAndHexString(hi,lo));

}

void BusMonitor::parseTxPDU(QStringList pdu)
{
    int ofs = 3; //start of message offset

    ui->txtPDU->setPlainText("Type : Tx Message");
    ui->txtPDU->appendPlainText("Timestamp : " + pdu[0]);

    if (pdu.length() < ofs+5){//check message length
        ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
        return;
    }

    bool ok;
    ui->txtPDU->appendPlainText("Slave Address : " + _toIntAndHexString("00",pdu[ofs]));

    int fcode = pdu[ofs+1].toInt(&ok,16);
    if(fcode <= 16)
    {
        ui->txtPDU->appendPlainText("Function : " + EUtils::ModbusFunctionName(fcode));
    }
    else
    {
        ui->txtPDU->appendPlainText("Function : Unknown [0x" + pdu[ofs+1] + "]");
        ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
        return;
    }

    ui->txtPDU->appendPlainText("Starting Address : " + _toIntAndHexString(pdu[ofs+2],pdu[ofs+3]));

    if (fcode == 1 || fcode == 2 || fcode == 3 || fcode == 4 || fcode == 15 || fcode == 16)
    {
        ui->txtPDU->appendPlainText("Quantity of Registers : " + _toIntAndHexString(pdu[ofs+4],pdu[ofs+5]));
    }
    else if (fcode == 5 || fcode == 6)
    {
        ui->txtPDU->appendPlainText("Output Value : " + _toIntAndHexString(pdu[ofs+4],pdu[ofs+5]));
    }

    if (fcode == 15 || fcode == 16){
        if (pdu.length() < ofs+7){
            ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
            return;
        }

        int byteCount = pdu[ofs+6].toInt(nullptr,16);
        ui->txtPDU->appendPlainText("Byte Count : " + _toIntAndHexString("00",pdu[ofs+6]));

        if (fcode == 16 && byteCount % 2 != 0){
            ui->txtPDU->appendPlainText("Error! Uneven number of bytes, cannot parse remainder of message");
            return;
        }

        if (pdu.length() < ofs+7+byteCount){//check message length
            ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
            return;
        }

        ui->txtPDU->appendPlainText("Output Values :");
        if(fcode == 16)
            for (int i = ofs+7; i < ofs+7+byteCount; i+=2)
            {
                ui->txtPDU->appendPlainText(QString::number((i-ofs-7)/2) + " : " + _toIntAndHexString(pdu[i],pdu[i+1]));
            }
        else
        {
            for (int i = ofs+7; i < ofs+7+byteCount; i++)
            {
                ui->txtPDU->appendPlainText(QString::number(i-ofs-7) + " : " + _toIntAndHexString("00",pdu[i]));
            }
        }
    }

}

void BusMonitor::parseRxPDU(QStringList pdu)
{
    int ofs = 3; //start of message offset

    ui->txtPDU->setPlainText("Type : Rx Message");
    ui->txtPDU->appendPlainText("Timestamp : " + pdu[0]);

    if (pdu.length() < ofs+2){//check message length
        ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
        return;
    }

    bool ok;
    ui->txtPDU->appendPlainText("Slave Address : " + _toIntAndHexString("00",pdu[ofs]));

    int fcode = pdu[ofs+1].toInt(&ok,16);
    if(fcode <= 16 || (fcode > 0x80 && fcode <= 0x80+16))
    {
        ui->txtPDU->appendPlainText("Function : " + EUtils::ModbusFunctionName(fcode));
    }
    else
    {
        ui->txtPDU->appendPlainText("Function : Unknown [0x" + pdu[ofs+1] + "]");
        ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
        return;
    }

    //if exception
    if (fcode >= 0x80)
    {
        fcode -= 0x80;

        if (pdu.length() < ofs+3){//check message length
            ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
            return;
        }

        int e = pdu[ofs+2].toInt(&ok,16);
        ui->txtPDU->appendPlainText("Exception : " + EUtils::ModbusExceptionName(e));
    }


    if (fcode <= 4){
        if (pdu.length() < ofs+3){
            ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
            return;
        }

        int byteCount = pdu[ofs+2].toInt(nullptr,16);
        ui->txtPDU->appendPlainText("Byte Count : " + _toIntAndHexString("00",pdu[ofs+2]));

        if ((fcode == 3 || fcode == 4) && byteCount % 2 != 0){
            ui->txtPDU->appendPlainText("Error! Uneven number of bytes, cannot parse remainder of message");
            return;
        }

        if (pdu.length() < ofs+4+byteCount){//check message length
            ui->txtPDU->appendPlainText("Error! Cannot parse remainder of message");
            return;
        }

        ui->txtPDU->appendPlainText("Output Values :");
        if(fcode == 3 || fcode == 4)
            for (int i = ofs+3; i < ofs+3+byteCount; i+=2)
            {
                ui->txtPDU->appendPlainText(QString::number((i-ofs-3)/2) + " : " + _toIntAndHexString(pdu[i],pdu[i+1]));
            }
        else
        {
            for (int i = ofs+3; i < ofs+3+byteCount; i++)
            {
                ui->txtPDU->appendPlainText(QString::number(i-ofs-3) + " : " + _toIntAndHexString("00",pdu[i]));
            }
        }
    }

    else if (fcode == 5 || fcode == 6){//write
        if (pdu.length() < ofs+6){//check message length
            ui->txtPDU->appendPlainText("Error! Cannot parse Message");
            return;
        }
        ui->txtPDU->appendPlainText("Starting Address : " + pdu[ofs+2] + pdu[ofs+3]);
        ui->txtPDU->appendPlainText("Output Value : " + pdu[ofs+4] + pdu[ofs+5]);
    }
    else if (fcode == 15 || fcode == 16){//write multiple
        if (pdu.length() < ofs+6){//check message length
            ui->txtPDU->appendPlainText("Error! Cannot parse Message");
            return;
        }
        ui->txtPDU->appendPlainText("Starting Address : " + pdu[ofs+2] + pdu[ofs+3]);
        ui->txtPDU->appendPlainText("Quantity of Registers : " + pdu[ofs+4] + pdu[ofs+5]);
    }
    else if (fcode > 0x80){//exception
        if (pdu.length() < ofs+3){//check message length
            ui->txtPDU->appendPlainText("Error! Cannot parse Message");
            return;
        }
        ui->txtPDU->appendPlainText("Exception Code : " + pdu[ofs+2]);
    }

}

void BusMonitor::parseSysMsg(QString msg)
{
    ui->txtPDU->setPlainText("Type : System Message");
    QStringList row = msg.split(QRegExp("\\s+"));
    ui->txtPDU->appendPlainText("Timestamp : " + row[2]);
    ui->txtPDU->appendPlainText("Message" + msg.mid(msg.indexOf(" : ")));
}


void BusMonitor::busRawTxData(uint8_t * data, uint8_t dataLen)
{
    //Raw Tx data from modbus port - Update raw data model
    QString line;

    for(int i = 0; i < dataLen; i++ ) {
        line += QString().sprintf( "%.2X  ", data[i] );
    }

    QLOG_INFO() << "Tx Data : " << line;
    line = EUtils::TxTimeStamp() + line;

    m_rawModel->appendRow(new QStandardItem(line));

}

void BusMonitor::busRawRxData(uint8_t * data, uint8_t dataLen)
{
    //Raw Rx data from modbus port - Update raw data model

    QString line;

    for(int i = 0; i < dataLen; i++ ) {
        line += QString().sprintf( "%.2X  ", data[i] );
    }

    QLOG_INFO() << "Rx Data : " << line;
    line = EUtils::RxTimeStamp() + line;

    m_rawModel->appendRow(new QStandardItem(line));
}



extern "C" {

void modbusRawRxData(uint8_t * data, uint8_t datalen)
{
   m_instance->busRawRxData(data, datalen);
}

void modbusRawTxData(uint8_t * data, uint8_t datalen)
{
   m_instance->busRawTxData(data, datalen);
}

}

