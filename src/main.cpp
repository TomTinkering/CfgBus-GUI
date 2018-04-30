#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <QDir>
#include <QTranslator>

#include "QsLog.h"
#include "QsLogDest.h"
#include "mainwindow.h"
#include "modbus.h"

QTranslator *Translator;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //init the logging mechanism
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::InfoLevel);
    const QString sLogPath(QDir(app.applicationDirPath()).filePath("QModMaster.log"));
    QsLogging::DestinationPtr fileDestination(QsLogging::DestinationFactory::MakeFileDestination(sLogPath,true,65535,2));
    QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);

    //show main window
    mainWin = new MainWindow(NULL);
    mainWin->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
    mainWin->show();

    return app.exec();

}
