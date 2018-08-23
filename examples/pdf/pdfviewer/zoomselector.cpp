/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "zoomselector.h"

#include <QLineEdit>

/*!
 * Constructs a new ZoomSelector with parent object \a parent.
 */
ZoomSelector::ZoomSelector(QWidget *parent)
    : QComboBox(parent)
{
    setEnabled(false);
    setEditable(true);

    addItem(QLatin1String("Fit Width"));
    addItem(QLatin1String("Fit Page"));
    addItem(QLatin1String("12%"));
    addItem(QLatin1String("25%"));
    addItem(QLatin1String("33%"));
    addItem(QLatin1String("50%"));
    addItem(QLatin1String("66%"));
    addItem(QLatin1String("75%"));
    addItem(QLatin1String("100%"));
    addItem(QLatin1String("125%"));
    addItem(QLatin1String("150%"));
    addItem(QLatin1String("200%"));
    addItem(QLatin1String("400%"));

    connect(this, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &ZoomSelector::onCurrentTextChanged);

    connect(lineEdit(), &QLineEdit::editingFinished,
            this, [this](){onCurrentTextChanged(lineEdit()->text()); });
}

/*!
 * Sets the text to \a zoomFactor * 100 with an % at the end.
 * Uses qRound() to get the an integer number.
 * Emits onCurrentTextChanged(QString) (If it only sets the text without emitting this signal, the zoom actions
 *                                      don't work properly after changing the zoom mode to "Fit Width" or "Fit Page".)
 */
void ZoomSelector::setZoomFactor(qreal zoomFactor)
{
    QString text = QString::number(qRound(zoomFactor * 100)) + QLatin1String("%");
    setCurrentText(text);
    emit onCurrentTextChanged(text);
}

/*!
 * Resets the zoomFactor to 1 (= 100%)
 */
void ZoomSelector::reset()
{
    setCurrentIndex(8); // 100%
}

/*!
 * Emits zoomModeChanged(QPdfView::ZoomMode) (and zoomFactorChanged(qreal) [if mode is custom])
 *
 * If the text equals "Fit Width" the mode will be QPdfView::FitToWidth.
 * If the text equals "Fit Page" the mode will be QPdfView::FitInView.
 * Else the mode will be QPdfView::CustomZoom and the zoomFactor is taken from the text field.
 */
void ZoomSelector::onCurrentTextChanged(const QString &text)
{
    if (text == QLatin1String("Fit Width")) {
        emit zoomModeChanged(QPdfView::FitToWidth);
    } else if (text == QLatin1String("Fit Page")) {
        emit zoomModeChanged(QPdfView::FitInView);
    } else {
        qreal factor = 1.0;

        QString withoutPercent(text);
        withoutPercent.remove(QLatin1Char('%'));

        bool ok = false;
        const int zoomLevel = withoutPercent.toInt(&ok);
        if (ok)
            factor = zoomLevel / 100.0;

        emit zoomModeChanged(QPdfView::CustomZoom);
        emit zoomFactorChanged(factor);
    }
}
