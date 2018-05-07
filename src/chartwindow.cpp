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
#include <math.h>

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
    m_ui->addAxis->addItem("Y-Axis A",axisA);
    m_ui->addAxis->addItem("Y-Axis B",axisB);

    m_chart->legend()->show();
    m_chart->setTitle("Acquired data from []");

    //setup chartview
    m_ui->chartView->setChart(m_chart);

    m_xStart = QDateTime::currentDateTime().toSecsSinceEpoch();
    m_xAxis->setTickCount(10);
    m_xAxis->setTitleText("Elapsed (seconds)");

    updateXAxis();

    m_yAxisA->setRange(-1, 1);
    m_yAxisA->setTitleText("Value Y-Axis A");

    m_yAxisB->setRange(-1, 1);
    m_yAxisB->setTitleText("Value Y-Axis B");

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

    connect(m_ui->actionAutoFitA,SIGNAL(triggered()),this,SLOT(autoFitA()));
    connect(m_ui->actionAutoFitB,SIGNAL(triggered()),this,SLOT(autoFitB()));
    connect(m_ui->actionClear,SIGNAL(triggered()),this,SLOT(chartReset()));

    chartReset();
}


void ChartWindow::autoFitA()
{
    auto autoMin = m_ui->yAMinAuto->checkState();
    auto autoMax = m_ui->yAMaxAuto->checkState();

    m_ui->yAMaxAuto->setCheckState(Qt::Checked);
    m_ui->yAMinAuto->setCheckState(Qt::Checked);

    updateYAAxis();

    m_ui->yAMaxAuto->setCheckState(autoMax);
    m_ui->yAMinAuto->setCheckState(autoMin);
}

void ChartWindow::autoFitB()
{
    auto autoMin = m_ui->yBMinAuto->checkState();
    auto autoMax = m_ui->yBMaxAuto->checkState();

    m_ui->yBMaxAuto->setCheckState(Qt::Checked);
    m_ui->yBMinAuto->setCheckState(Qt::Checked);

    updateYBAxis();

    m_ui->yBMaxAuto->setCheckState(autoMax);
    m_ui->yBMinAuto->setCheckState(autoMin);
}


void ChartWindow::chartReset()
{
    m_ui->addSeries->clear();
    m_ui->removeSeries->clear();

    m_chart->removeAllSeries();
    m_plots.clear();

    m_ui->yAMinAuto->setChecked(Qt::Checked);
    m_ui->yAMaxAuto->setChecked(Qt::Checked);
    m_ui->yBMinAuto->setChecked(Qt::Checked);
    m_ui->yBMaxAuto->setChecked(Qt::Checked);

    m_ui->yAMin->setEnabled(false);
    m_ui->yAMax->setEnabled(false);
    m_ui->yBMin->setEnabled(false);
    m_ui->yBMax->setEnabled(false);

    autoFitA();
    autoFitB();

    m_ui->addSeries->addItems(m_availableSeries);
    m_ui->addSeries->model()->sort(0);
}


ChartWindow::MinMax ChartWindow::getAxisMinMax(YAxis axis)
{
    MinMax result;
    result.yMax = std::numeric_limits<qreal>::lowest();
    result.yMin = std::numeric_limits<qreal>::max();
    result.xMin = m_xStart;
    result.xMax = m_xStart;

    auto currentXMin = m_xAxis->min();

    for(auto plot : m_plots)
    {
        if(plot.axis == axis)
        {
            auto minMax = plot.minMax;

            //if series minmax is within current x-axis range, update yMin/yMax
            if(minMax.xMin > currentXMin || minMax.xMax > currentXMin)
            {
                if(minMax.yMin < result.yMin)
                {
                    result.yMin = minMax.yMin;
                    result.xMin = minMax.xMin;
                }

                if(minMax.yMax > result.yMax)
                {
                    result.yMax = minMax.yMax;
                    result.xMax = minMax.xMax;
                }
            }

        }
    }

    if(result.yMin == std::numeric_limits<qreal>::max() || result.yMax == std::numeric_limits<qreal>::lowest())
    {
        result.yMin = -1;
        result.yMax = 1;
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
        m_yAMin = minMax.yMin;
        m_yAMax = minMax.yMax;

        if(autoMin)
            m_ui->yAMin->setValue(m_yAMin);

        if(autoMax)
            m_ui->yAMax->setValue(m_yAMax);
    }

    qreal min = (autoMin) ? m_yAMin : m_ui->yAMin->value();
    qreal max = (autoMax) ? m_yAMax : m_ui->yAMax->value();

    //auto calc some additional range if automatic axis
    min -= (autoMin) ? abs(max-min)*0.05 : 0;
    max += (autoMax) ? abs(max-min)*0.05 : 0;

    if(min == max)
    {
        min -= 0.001;
        max += 0.001;
    }

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
        m_yBMin = minMax.yMin;
        m_yBMax = minMax.yMax;

        if(autoMin)
            m_ui->yBMin->setValue(m_yBMin);

        if(autoMax)
            m_ui->yBMax->setValue(m_yBMax);
    }

    qreal min = (autoMin) ? m_yBMin : m_ui->yBMin->value();
    qreal max = (autoMax) ? m_yBMax : m_ui->yBMax->value();

    //auto calc some additional range if automatic axis
    min -= (autoMin) ? abs(max-min)*0.05 : 0;
    max += (autoMax) ? abs(max-min)*0.05 : 0;

    if(min == max)
    {
        min -= 0.001;
        max += 0.001;
    }

    //update axis if necessary
    if(m_yAxisB->min() != min || m_yAxisB->max() != max)
        m_yAxisB->setRange(min,max);
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


void ChartWindow::yAMinChanged()
{
    auto min = m_ui->yAMin->value();
    auto max = m_ui->yAMax->value();

    bool autoMax = m_ui->yAMaxAuto->checkState() == Qt::Checked;

    if(min >= max)
    {
        if(autoMax)
        {
            min = max - 0.001;
            m_ui->yAMin->setValue(min);
        }
        else
        {
            max = min + 0.001;
            m_ui->yAMax->setValue(max);
        }
    }

    updateYAAxis();
}

void ChartWindow::yAMaxChanged()
{
    auto min = m_ui->yAMin->value();
    auto max = m_ui->yAMax->value();

    bool autoMin = m_ui->yAMinAuto->checkState() == Qt::Checked;

    if(min >= max)
    {
        if(autoMin)
        {
            max = min + 0.001;
            m_ui->yAMax->setValue(max);
        }
        else
        {
            min = max - 0.001;
            m_ui->yAMin->setValue(min);
        }
    }

    updateYAAxis();
}

void ChartWindow::yBMinChanged()
{
    auto min = m_ui->yBMin->value();
    auto max = m_ui->yBMax->value();

    bool autoMax = m_ui->yBMaxAuto->checkState() == Qt::Checked;

    if(min >= max)
    {
        if(autoMax)
        {
            min = max - 0.001;
            m_ui->yBMin->setValue(min);
        }
        else
        {
            max = min + 0.001;
            m_ui->yBMax->setValue(max);
        }
    }

    updateYBAxis();
}

void ChartWindow::yBMaxChanged()
{
    auto min = m_ui->yBMin->value();
    auto max = m_ui->yBMax->value();

    bool autoMin = m_ui->yBMinAuto->checkState() == Qt::Checked;

    if(min >= max)
    {
        if(autoMin)
        {
            max = min + 0.001;
            m_ui->yBMax->setValue(max);
        }
        else
        {
            min = max - 0.001;
            m_ui->yBMin->setValue(min);
        }
    }

    updateYBAxis();
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
        updateYBAxis();
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
        updateYBAxis();
    } //if not
    else
    {
        m_ui->yBMax->setEnabled(true);
    }
}


void ChartWindow::removePlot()
{
    auto seriesName = m_ui->removeSeries->currentText();

    if(m_plots.empty() || !m_plots.contains(seriesName) || seriesName == "")
        return;

    m_chart->removeSeries(m_plots[seriesName].series);
    m_plots.remove(seriesName);

    //move seriesname from remove to add combobox
    m_ui->addSeries->addItem(seriesName);
    m_ui->removeSeries->removeItem(m_ui->removeSeries->currentIndex());
    m_ui->addSeries->model()->sort(0);
}


void ChartWindow::addPlot()
{
    auto seriesName = m_ui->addSeries->currentText();

    if(m_plots.contains(seriesName) || seriesName == "")
        return;

    PlotAdmin plot;

    plot.series = new QLineSeries();
    plot.series->setName(seriesName);

    plot.axis = static_cast<YAxis>(m_ui->addAxis->currentData().toInt());

    MinMax minMax;
    minMax.yMax = std::numeric_limits<qreal>::lowest();
    minMax.yMin = std::numeric_limits<qreal>::max();
    minMax.xMin = m_xStart;
    minMax.xMax = m_xStart;

    plot.minMax = minMax;

    m_plots.insert(seriesName,plot);

    //move seriesname from add to remove combobox
    m_ui->removeSeries->addItem(seriesName);
    m_ui->addSeries->removeItem(m_ui->addSeries->currentIndex());
    m_ui->removeSeries->model()->sort(0);

    //setup chart
    m_chart->addSeries(plot.series);
    m_chart->setAxisX(m_chart->axisX(), plot.series);

    switch(plot.axis)
    {
        case axisA:
            m_chart->setAxisY(m_yAxisA, plot.series);
            break;
        case axisB:
            m_chart->setAxisY(m_yAxisB,plot.series);
            break;
    }
}

void ChartWindow::setSeriesNames(QStringList seriesNames)
{
    m_availableSeries = seriesNames;
    chartReset();
}

void ChartWindow::enableControls(bool enabled)
{
    m_ui->addSeries->clear();
    m_ui->removeSeries->clear();

    m_ui->addSeries->setEnabled(enabled);
    m_ui->addBtn->setEnabled(enabled);
    m_ui->addAxis->setEnabled(enabled);

    m_ui->removeBtn->setEnabled(enabled);
    m_ui->removeSeries->setEnabled(enabled);

    m_ui->yAMin->setEnabled(enabled);
    m_ui->yAMax->setEnabled(enabled);
    m_ui->yAMinAuto->setEnabled(enabled);
    m_ui->yAMaxAuto->setEnabled(enabled);

    m_ui->yBMin->setEnabled(enabled);
    m_ui->yBMax->setEnabled(enabled);
    m_ui->yBMinAuto->setEnabled(enabled);
    m_ui->yBMaxAuto->setEnabled(enabled);

    m_ui->xRange->setEnabled(enabled);

    m_ui->actionAutoFitA->setEnabled(enabled);
    m_ui->actionAutoFitB->setEnabled(enabled);
    m_ui->actionClear->setEnabled(enabled);
}



void ChartWindow::addDataPoint(QString seriesName, qreal point)
{
    if(m_plots.empty() || !m_plots.contains(seriesName))
        return;

    //update x-axis
    this->updateXAxis();

    auto series = m_plots[seriesName].series;

    //build point
    qreal now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qreal elapsed = now - m_xStart*1000;
    elapsed /= 1000;

    series->append(elapsed,point);

    //remove samples that are below x-axis minimum
    //and calculate y-axis if set to auto
    auto points = series->pointsVector();

    //remove points outside of axis limits
    while(!points.empty() && points[0].x() < elapsed-m_xNrSeconds)
    {
        //do not remove point if its the only point outside of axis range
        if(points.count() >= 2 && points[1].x() >= elapsed-m_xNrSeconds)
            break;

        points.remove(0);
    }

    series->replace(points);

    //always keep track of min/avg/max for auto features
    MinMax minMax;
    minMax.yMax = std::numeric_limits<qreal>::lowest();
    minMax.yMin = std::numeric_limits<qreal>::max();
    minMax.xMin = m_xStart;
    minMax.xMax = m_xStart;

    m_plots[seriesName].avg = 0;

    for(int i=0; i<points.count(); i++)
    {
        m_plots[seriesName].avg += points[i].y();

        if (points[i].y() <= minMax.yMin)
        {
            minMax.yMin = points[i].y();
            minMax.xMin = points[i].x();
        }

        if (points[i].y() >= minMax.yMax)
        {
            minMax.yMax = points[i].y();
            minMax.xMax = points[i].x();
        }
    }

    m_plots[seriesName].minMax = minMax;
    m_plots[seriesName].avg /= points.count();

    switch(m_plots[seriesName].axis)
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

