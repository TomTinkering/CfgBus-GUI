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
#include <QDateTime>

#include "chartwindow.h"
#include "ui_chartwindow.h"
#include <algorithm>
#include <tuple>

QT_CHARTS_USE_NAMESPACE

ChartWindow::ChartWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::ChartWindow),
      m_chart(new QChart),
      m_xAxis(new QValueAxis),
      m_yAxisA(new QValueAxis),
      m_yAxisB(new QValueAxis)
{
    //setup UI
    m_ui->setupUi(this);

    m_chart->legend()->show();
    m_chart->setTitle("Acquired data from []");

    //setup chartview
    m_ui->chartView->setChart(m_chart);

    m_xStart = QDateTime::currentDateTime().toSecsSinceEpoch();
    m_xAxis->setTickCount(10);
    //m_xAxis->setFormat("hh:mm:ss");
    m_xAxis->setTitleText("Time (hh:mm:ss)");

    updateXAxis();

    m_yAxisA->setRange(-1, 1);
    m_yAxisA->setTitleText("Value");

    m_yAxisB->setRange(-1, 1);
    m_yAxisB->setTitleText("Value");

    m_chart->addAxis(m_xAxis,Qt::AlignBottom);
    m_chart->addAxis(m_yAxisA,Qt::AlignLeft);
    m_chart->addAxis(m_yAxisB,Qt::AlignRight);

    //set initial values number boxes
    m_ui->yAMin->setMinimum(std::numeric_limits<double>::lowest());
    m_ui->yAMin->setMaximum(std::numeric_limits<double>::max());
    m_ui->yAMax->setMinimum(std::numeric_limits<double>::lowest());
    m_ui->yAMax->setMaximum(std::numeric_limits<double>::max());
    m_ui->yBMin->setMinimum(std::numeric_limits<double>::lowest());
    m_ui->yBMin->setMaximum(std::numeric_limits<double>::max());
    m_ui->yBMax->setMinimum(std::numeric_limits<double>::lowest());
    m_ui->yBMax->setMaximum(std::numeric_limits<double>::max());
    m_ui->xRange->setMinimum(0.1);
    m_ui->xRange->setMaximum(std::numeric_limits<double>::max());

    m_ui->yAMin->setValue(-1);
    m_ui->yAMax->setValue(1);
    m_ui->yBMin->setValue(-1);
    m_ui->yBMax->setValue(1);
    m_ui->xRange->setValue(m_xNrSeconds);

    //connect UI elements
    connect(m_ui->addBtn,SIGNAL(released()),this,SLOT(addPlot()));
    connect(m_ui->removeBtn,SIGNAL(released()),this,SLOT(removePlot()));

    connect(m_ui->yAMin,SIGNAL(editingFinished()),this,SLOT(yAMinChanged()));
    connect(m_ui->yAMax,SIGNAL(editingFinished()),this,SLOT(yAMaxChanged()));
    connect(m_ui->yAMinAuto,SIGNAL(released()),this,SLOT(yAMinAutoChanged()));
    connect(m_ui->yAMaxAuto,SIGNAL(released()),this,SLOT(yAMaxAutoChanged()));

    connect(m_ui->yBMin,SIGNAL(editingFinished()),this,SLOT(yBMinChanged()));
    connect(m_ui->yBMax,SIGNAL(editingFinished()),this,SLOT(yBMaxChanged()));
    connect(m_ui->yBMinAuto,SIGNAL(released()),this,SLOT(yBMinAutoChanged()));
    connect(m_ui->yBMaxAuto,SIGNAL(released()),this,SLOT(yBMaxAutoChanged()));

    connect(m_ui->xRange,SIGNAL(editingFinished()),this,SLOT(xRangeChanged()));
}

ChartWindow::MinMax ChartWindow::getAxisMinMax(YAxis axis)
{
    MinMax result;

    if (m_series.empty())
        return result;

    result.min = std::numeric_limits<double>::max();
    result.max = std::numeric_limits<double>::lowest();

    for(auto series : m_series)
    {
        auto name = series->name();

        if(m_seriesAxes[name] == axis)
        {
            result.min = (m_seriesMin[name] < result.min) ? m_seriesMin[name] : result.min;
            result.max = (m_seriesMax[name] > result.max) ? m_seriesMax[name] : result.max;
        }
    }

    if(result.min == std::numeric_limits<double>::max() || result.max == std::numeric_limits<double>::lowest())
    {
        result.min = -1;
        result.max = 1;
    }

    return result;
}

void ChartWindow::updateYAAxis()
{
    bool autoMin = m_ui->yAMinAuto->checkState() == Qt::Checked;
    bool autoMax = m_ui->yAMaxAuto->checkState() == Qt::Checked;

    //if this axis is (partly) auto, determine min max

    if(autoMin || autoMax)
    {
        auto minMax = getAxisMinMax(axisA);
        m_yAMin = minMax.min;
        m_yAMax = minMax.max;

        if(autoMin)
            m_ui->yAMin->setValue(m_yAMin);

        if(autoMax)
            m_ui->yAMax->setValue(m_yAMax);
    }

    qreal min = (autoMin) ? m_yAMin : m_ui->yAMin->value();
    qreal max = (autoMax) ? m_yAMax : m_ui->yAMax->value();

    //always leave some room at top and bottom of chart
    min -= (max-min)*0.05;
    max += (max-min)*0.05;

    //update axis if necessary
    if(m_yAxisA->min() != min || m_yAxisA->max() != max)
        m_yAxisA->setRange(min,max);
}

void ChartWindow::updateYBAxis()
{
    bool autoMin = m_ui->yBMinAuto->checkState() == Qt::Checked;
    bool autoMax = m_ui->yBMaxAuto->checkState() == Qt::Checked;

    //if this axis is (partly) auto, determine min max

    if(autoMin || autoMax)
    {
        auto minMax = getAxisMinMax(axisB);
        m_yBMin = minMax.min;
        m_yBMax = minMax.max;

        if(autoMin)
            m_ui->yBMin->setValue(m_yBMin);

        if(autoMax)
            m_ui->yBMax->setValue(m_yBMax);
    }

    qreal min = (autoMin) ? m_yBMin : m_ui->yBMin->value();
    qreal max = (autoMax) ? m_yBMax : m_ui->yBMax->value();

    //always leave some room at top and bottom of chart
    min -= (max-min)*0.05;
    max += (max-min)*0.05;

    //update axis if necessary
    if(m_yAxisA->min() != min || m_yAxisA->max() != max)
        m_yAxisA->setRange(min,max);
}


void ChartWindow::xRangeChanged()
{
    m_xNrSeconds = m_ui->xRange->value();
    updateXAxis();
}


void ChartWindow::updateXAxis()
{
    qreal now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qreal elapsed = now - m_xStart*1000;
    elapsed /= 1000;
    m_xAxis->setRange(elapsed-m_xNrSeconds,elapsed);
}

void ChartWindow::updateAxis()
{
    updateYAAxis();
    updateYBAxis();
    updateXAxis();
}

void ChartWindow::yAMinChanged()
{
    auto valMin = m_ui->yAMin->value();
    auto valMax = m_ui->yAMax->value();

    if(valMin > valMax)
        m_ui->yAMin->setValue(valMax - 0.01);

    updateYAAxis();
}

void ChartWindow::yAMaxChanged()
{
    auto valMin = m_ui->yAMin->value();
    auto valMax = m_ui->yAMax->value();

    if(valMin > valMax)
        m_ui->yAMax->setValue(valMin + 0.01);

    updateYAAxis();
}

void ChartWindow::yBMinChanged()
{
    auto valMin = m_ui->yBMin->value();
    auto valMax = m_ui->yBMax->value();

    if(valMin > valMax)
        m_ui->yBMin->setValue(valMax - 0.01);

    updateYAAxis();
}

void ChartWindow::yBMaxChanged()
{
    auto valMin = m_ui->yBMin->value();
    auto valMax = m_ui->yBMax->value();

    if(valMin > valMax)
        m_ui->yBMax->setValue(valMin + 0.01);

    updateYAAxis();
}

void ChartWindow::yAMinAutoChanged()
{
    //if we are now automatically determining lower limit
    if(m_ui->yAMinAuto->checkState() == Qt::Checked)
    {
        m_ui->yAMin->setEnabled(false);
        m_ui->yAMin->setValue(m_yAMin);
        updateYAAxis();
    } //if not
    else
    {
        m_ui->yAMin->setEnabled(true);
    }
}

void ChartWindow::yAMaxAutoChanged()
{
    //if we are now automatically determining lower limit
    if(m_ui->yAMaxAuto->checkState() == Qt::Checked)
    {
        m_ui->yAMax->setEnabled(false);
        m_ui->yAMax->setValue(m_yAMax);
        updateYAAxis();
    } //if not
    else
    {
        m_ui->yAMax->setEnabled(true);
    }
}


void ChartWindow::yBMinAutoChanged()
{
    //if we are now automatically determining lower limit
    if(m_ui->yBMinAuto->checkState() == Qt::Checked)
    {
        m_ui->yBMin->setEnabled(false);
        m_ui->yBMin->setValue(m_yBMin);
        updateYAAxis();
    } //if not
    else
    {
        m_ui->yBMin->setEnabled(true);
    }
}

void ChartWindow::yBMaxAutoChanged()
{
    //if we are now automatically determining lower limit
    if(m_ui->yBMaxAuto->checkState() == Qt::Checked)
    {
        m_ui->yBMax->setEnabled(false);
        m_ui->yBMax->setValue(m_yBMax);
        updateYAAxis();
    } //if not
    else
    {
        m_ui->yBMax->setEnabled(true);
    }
}


void ChartWindow::removePlot(QString seriesName)
{
    if(m_series.empty() || !m_series.contains(seriesName))
        return;

    m_chart->removeSeries(m_series.value(seriesName));
    m_series.remove(seriesName);
    m_seriesAxes.remove(seriesName);
    m_seriesMin.remove(seriesName);
    m_seriesMax.remove(seriesName);
}


void ChartWindow::addPlot(QString seriesName)
{
    if(m_series.contains(seriesName))
        return;

    auto series = new QLineSeries();
    series->setName(seriesName);
    m_series.insert(seriesName, series);
    m_seriesAxes.insert(seriesName,axisA);
    m_seriesMin.insert(seriesName,-1);
    m_seriesMax.insert(seriesName,1);

    //setup chart
    m_chart->addSeries(series);
    m_chart->setAxisX(m_chart->axisX(), series);
    m_chart->setAxisY(m_chart->axisY(), series);
}


void ChartWindow::addDataPoint(QString seriesName, qreal point)
{
    //update x-axis
    this->updateXAxis();

    if(m_series.empty() || !m_series.contains(seriesName))
        addPlot(seriesName);

    auto series = m_series.value(seriesName);

    //build point
    qreal now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qreal elapsed = now - m_xStart*1000;
    elapsed /= 1000;

    series->append(elapsed,point);

    //remove samples that are below x-axis minimum
    //and calculate y-axis if set to auto
    auto points = series->pointsVector();

    //remove points outside of axis limits + 1s
    while(!points.empty() && points[0].x() < (elapsed-(m_xNrSeconds+1)))
    {
        points.remove(0);
    }

    if(points.empty())
        return;

    //always keep track of min/max for auto features
    qreal min = std::numeric_limits<qreal>::max();
    qreal max = std::numeric_limits<qreal>::min();

    for(int i=0; i<points.count(); i++)
    {
        if (points[i].y() < min)
            min = points[i].y();

        if (points[i].y() > max)
            max = points[i].y();
    }

    m_seriesMin[seriesName] = min;
    m_seriesMax[seriesName] = max;

    switch(m_seriesAxes.value(seriesName))
    {
        case axisA:
            this->updateYAAxis();
            break;

        case axisB:
            this->updateYBAxis();
            break;
    }

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

