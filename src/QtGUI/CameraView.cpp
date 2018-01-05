/*                         C A M E R A V I E W . C P P
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file CameraView.cpp
 *
 *  BRL-CAD Qt GUI:
 *      implementation projection control view
 */

#include <limits>
#include <cmath>

#include <QGridLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QPushButton>

#include "CameraView.h"


CameraView::CameraView
(
    BRLCAD::ConstDatabase& database,
    QWidget*               graphicView,
    QWidget*               parent
) : QWidget(parent), m_database(database), m_graphicView(graphicView) {
    m_timer = new QTimer;
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &CameraView::TimeOut);

    QGridLayout* mainLayout       = new QGridLayout(this);
    QLabel*      translationLabel = new QLabel(tr("Translation"));
    mainLayout->addWidget(translationLabel, 0, 0, 1, -1);

    QLabel* xEditLabel = new QLabel("x:");
    QLabel* yEditLabel = new QLabel("y:");
    QLabel* zEditLabel = new QLabel("z:");
    m_x = new QLineEdit("0");
    m_y = new QLineEdit("0");
    m_z = new QLineEdit("0");
    m_x->setValidator(new QDoubleValidator);
    m_y->setValidator(new QDoubleValidator);
    m_z->setValidator(new QDoubleValidator);
    connect(m_x, SIGNAL(textEdited(const QString&)), m_timer, SLOT(start(void)));
    connect(m_y, SIGNAL(textEdited(const QString&)), m_timer, SLOT(start(void)));
    connect(m_z, SIGNAL(textEdited(const QString&)), m_timer, SLOT(start(void)));
    mainLayout->addWidget(xEditLabel, 1, 0);
    mainLayout->addWidget(m_x, 1, 1);
    mainLayout->addWidget(yEditLabel, 1, 2);
    mainLayout->addWidget(m_y, 1, 3);
    mainLayout->addWidget(zEditLabel, 1, 4);
    mainLayout->addWidget(m_z, 1, 5);

    QLabel* zoomLabel = new QLabel(tr("Zoom"));
    mainLayout->addWidget(zoomLabel, 2, 0, 1, -1);

    m_scale = new QLineEdit("1");
    m_scale->setValidator(new QDoubleValidator(std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::max(), 16));
    connect(m_scale, SIGNAL(textEdited(const QString&)), m_timer, SLOT(start(void)));
    mainLayout->addWidget(m_scale, 3, 1);

    QLabel* rotationLabel = new QLabel(tr("Rotation"));
    mainLayout->addWidget(rotationLabel, 4, 0, 1, -1);

    m_xDial = new QDial;
    m_xDial->setWrapping(true);
    m_xDial->setMinimum(0);
    m_xDial->setMaximum(359);
    m_yDial = new QDial;
    m_yDial->setWrapping(true);
    m_yDial->setMinimum(0);
    m_yDial->setMaximum(359);
    m_zDial = new QDial;
    m_zDial->setWrapping(true);
    m_zDial->setMinimum(0);
    m_zDial->setMaximum(359);
    connect(m_xDial, SIGNAL(valueChanged(int)), m_timer, SLOT(start(void)));
    connect(m_yDial, SIGNAL(valueChanged(int)), m_timer, SLOT(start(void)));
    connect(m_zDial, SIGNAL(valueChanged(int)), m_timer, SLOT(start(void)));
    mainLayout->addWidget(m_xDial, 5, 0, 1, 2);
    mainLayout->addWidget(m_yDial, 5, 2, 1, 2);
    mainLayout->addWidget(m_zDial, 5, 4, 1, 2);

    QPushButton* resetButton = new QPushButton(tr("Reset"));
    connect(resetButton, &QPushButton::clicked, this, &CameraView::Reset);
    mainLayout->addWidget(resetButton, 6, 0, 1, -1);
}


void CameraView::Reset(void) {
    BRLCAD::Vector3D dbMin          = m_database.BoundingBoxMinima();
    BRLCAD::Vector3D dbMax          = m_database.BoundingBoxMaxima();
    double           dbDiagMinSize  = sqrt(dbMin.coordinates[0] * dbMin.coordinates[0] + dbMin.coordinates[1] * dbMin.coordinates[1] + dbMin.coordinates[2] * dbMin.coordinates[2]);
    double           dbDiagMaxSize  = sqrt(dbMax.coordinates[0] * dbMax.coordinates[0] + dbMax.coordinates[1] * dbMax.coordinates[1] + dbMax.coordinates[2] * dbMax.coordinates[2]);
    double           dbMaxExtension = std::max(dbDiagMinSize, dbDiagMaxSize);
    double           dbXCenter      = (dbMin.coordinates[0] + dbMax.coordinates[0]) / 2.;
    double           dbYCenter      = (dbMin.coordinates[1] + dbMax.coordinates[1]) / 2.;
    double           dbXSize        = dbMax.coordinates[0] - dbMin.coordinates[0];
    double           dbYSize        = dbMax.coordinates[1] - dbMin.coordinates[1];
    double           imageWidth     = m_graphicView->width();
    double           imageHeight    = m_graphicView->height();
    double           imageXCenter   = imageWidth / 2.;
    double           imageYCenter   = imageHeight / 2.;
    double           scale          = 1.;

    if ((imageWidth > 0) && (dbXSize > std::numeric_limits<double>::epsilon())) {
        scale = dbXSize / imageWidth;

        if ((imageHeight > 0) && (dbYSize > std::numeric_limits<double>::epsilon())) {
            double scaley = dbYSize / imageHeight;

            if (scaley > scale)
                scale = scaley;
        }
    }
    else if ((imageHeight > 0) && (dbYSize > std::numeric_limits<double>::epsilon()))
        scale = dbYSize / imageHeight;

    m_x->setText(QString::number(dbXCenter - scale * imageXCenter));
    m_y->setText(QString::number(dbYCenter - scale * imageYCenter));
    m_z->setText(QString::number(dbMaxExtension + 1.));
    m_scale->setText(QString::number(scale));
    m_xDial->setSliderPosition(0);
    m_yDial->setSliderPosition(0);
    m_zDial->setSliderPosition(0);

    m_timer->start();
}


void CameraView::TimeOut(void) {
    QMatrix4x4 transformation;

    transformation.translate(m_x->text().toDouble(), m_y->text().toDouble(), m_z->text().toDouble());
    transformation.scale(m_scale->text().toDouble());
    transformation.rotate(m_xDial->value(), 1., 0., 0.);
    transformation.rotate(m_yDial->value(), 0., 1., 0.);
    transformation.rotate(m_zDial->value(), 0., 0., 1.);

    emit Changed(transformation);
}