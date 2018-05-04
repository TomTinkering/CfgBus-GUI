/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QTimer>
#include <QString>
#include <QMap>
#include <QStringList>
#include <QDateTime>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
class QChartView;
class QValueAxis;
class QAbstractAxis;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Ui {
    class ChartWindow;
}

class ChartWindow : public QMainWindow
{
    Q_OBJECT

public:
    ChartWindow(QWidget *parent = 0);
    ~ChartWindow();

    void reset();
    void setSeriesNames(QStringList seriesNames);
    void enableControls(bool en);
    void addDataPoint(QString plotName, qreal point);

    enum YAxis
    {
        axisA,
        axisB
    };

private:
    void updateYAAxis();
    void updateYBAxis();
    void updateXAxis();
    void updateAxis();

    struct MinMax
    {
        qreal xMin;
        qreal xMax;
        qreal yMin;
        qreal yMax;
    };

    struct PlotAdmin
    {
        QLineSeries *series;
        YAxis axis;
        MinMax minMax;
        qreal avg;
    };

    MinMax getAxisMinMax(YAxis axis);

    //chart
    Ui::ChartWindow *m_ui;
    QChart *m_chart;

    //axis
    QValueAxis *m_xAxis;
    QValueAxis *m_yAxisA;
    QValueAxis *m_yAxisB;
    double m_xNrSeconds = 10;
    qint64 m_xStart;

    qreal m_yAMax = 1;
    qreal m_yBMax = 1;
    qreal m_yAMin = -1;
    qreal m_yBMin = -1;

    //series
    QStringList m_availableSeries;
    QMap<QString,PlotAdmin> m_plots;
//    QMap<QString,QLineSeries *> m_series;
//    QMap<QString,YAxis> m_seriesAxes;
//    QMap<QString,MinMax> m_seriesMinMax;


protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void autoFitA();
    void autoFitB();
    void chartReset();
    void removePlot();
    void addPlot();
    void xRangeChanged();
    void yAMinChanged();
    void yAMaxChanged();
    void yAMinAutoChanged();
    void yAMaxAutoChanged();
    void yBMinChanged();
    void yBMaxChanged();
    void yBMinAutoChanged();
    void yBMaxAutoChanged();
};

#endif // CHARTWINDOW_H
