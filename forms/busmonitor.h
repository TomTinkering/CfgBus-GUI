#ifndef BUSMONITOR_H
#define BUSMONITOR_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QLabel>

namespace Ui {
    class BusMonitor;
}

class BusMonitor : public QMainWindow
{
    Q_OBJECT

public:
    explicit BusMonitor(QWidget *parent);
    ~BusMonitor();

    void busRawRxData(uint8_t * data, uint8_t dataLen);
    void busRawTxData(uint8_t * data, uint8_t dataLen);

    void setMaxLines(int maxLines)
    {
        m_MaxLines = maxLines;
        if(m_rawModel->rowCount() > m_MaxLines)
            m_rawModel->removeRows(0,m_rawModel->rowCount()-m_MaxLines);
    }

private:
    Ui::BusMonitor *ui;
    void parseMsg(QString msg);
    void parseTxPDU(QStringList pdu);
    void parseRxPDU(QStringList pdu);
    void parseSysMsg(QString msg);

    QStandardItemModel* m_rawModel;

    int m_MaxLines = 1000;

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void clear();
    void exit();
    void save();
    void selectedRow(const QModelIndex & selected);

};

#endif // BUSMONITOR_H
