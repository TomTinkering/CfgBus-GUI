#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QLabel>
#include <QString>
#include <QProcess>

#include "about.h"
#include "busmonitor.h"
#include "settingsdialog.h"
#include "cfgbusmaster.h"
#include "cfgbusmodel.h"
#include "chartwindow.h"
#include "MyInfoBar.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showUpInfoBar(QString message, MyInfoBar::InfoType type);
    void hideInfoBar();

private:
    Ui::MainWindow *ui;
    CfgBusModel *m_model;

    //UI - Dialogs
    About *m_dlgAbout;
    SettingsDialog *m_dlgSettings;
    BusMonitor *m_busMonitor;
    ChartWindow *m_chartWindow;

    //timer related
    QTimer *m_timer;
    int m_scanRate = 100;

    void updateStatusBar();
    QLabel *m_statusText;
    QLabel *m_statusInd;
    QLabel *m_statusPackets;
    QLabel *m_statusErrors;
    void modbusConnect(bool connect);

    void changeEvent(QEvent* event);

private slots:
    void plotEntryData(const CfgBusEntry* entry);
    void showSettings();
    void showBusMonitor();
    void changedScanRate(int value);
    void changedConnect(bool value);
    void clearItems();
    void getEntryList();
    void openLogFile();
    void scan(bool value);
    void request();
    void packetsUpdated();

signals:
    void resetCounters();

};

extern MainWindow *mainWin;

#endif // MAINWINDOW_H
