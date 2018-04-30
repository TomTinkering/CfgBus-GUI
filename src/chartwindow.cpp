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

#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioInput>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtWidgets/QVBoxLayout>
#include <QtCharts/QValueAxis>
#include <QTimer>

#include "chartwindow.h"
#include "ui_chartwindow.h"

QT_CHARTS_USE_NAMESPACE

ChartWindow::ChartWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::ChartWindow),
      m_chart(new QChart),
      m_series(0),
      m_timer(new QTimer())
{
    //setup UI
    m_ui->setupUi(this);

    //setup chart
    m_series.push_back(new QLineSeries);
    m_series.push_back(new QLineSeries);
    m_series[Series0]->setName("Series0");
    m_series[Series1]->setName("Series1");
    m_chart->addSeries(m_series[Series0]);
    m_chart->addSeries(m_series[Series1]);

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 100);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Samples [n]");

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-1, 1);
    axisY->setTitleText("Value");

    m_chart->setAxisX(axisX, m_series[Series0]);
    m_chart->setAxisY(axisY, m_series[Series0]);

    m_chart->setAxisX(axisX, m_series[Series1]);
    m_chart->setAxisY(axisY, m_series[Series1]);

    m_chart->legend()->show();
    m_chart->setTitle("Acquired data from []");

    //setup chartview
    m_ui->chartView->setChart(m_chart);

    //setup timer to generator random data
    m_timer = new QTimer();
    connect(m_timer,SIGNAL(timeout()),this,SLOT(genData()));
    m_timer->start(100);

}


void ChartWindow::genData()
{
    int rand0 = (qrand() / (RAND_MAX/100)) - 50;
    int rand1 = (qrand() / (RAND_MAX/100)) - 50;

    addDataPoint(Series0,rand0);
    addDataPoint(Series1,rand1);
}


void ChartWindow::addDataPoint(int series, float point)
{
    static uint32_t sample = 0;

    if(m_series.empty() || series >= m_series.size())
        return;

    qint64 range = 100;
    auto qSeries = m_series[series];

    sample++;
    qSeries->append(sample,point);

    if (qSeries->count() >= range)
    {
        qSeries->remove(0);
    }


    //calculate axis
    auto points = qSeries->pointsVector();
    float min = points[0].y();
    float max = points[0].y();

    for(int i=0; i<points.count(); i++)
    {
        auto newY = points[i].y();
        if (newY < min)
            min = newY;

        if (newY > max)
            max = newY;
    }

    //update axis
    m_chart->axisX(qSeries)->setMin(sample-range);
    m_chart->axisX(qSeries)->setMax(sample);
    m_chart->axisY(qSeries)->setMin(min-10);
    m_chart->axisY(qSeries)->setMax(max+10);

    return;
}


void ChartWindow::closeEvent(QCloseEvent *event)
{
   event->accept();
}

void ChartWindow::showEvent(QShowEvent *event)
{
    event->accept();
}

ChartWindow::~ChartWindow()
{
    delete m_ui;
}

