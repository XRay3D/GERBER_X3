// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "doublespinbox.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QTimer>

DoubleSpinBox::DoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent)
{
    lineEdit()->installEventFilter(this);
    setToolTipDuration(0);
}

void DoubleSpinBox::setRange(double min, double max)
{
    QDoubleSpinBox::setRange(min, max);
    setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::setMaximum(double max)
{
    QDoubleSpinBox::setMaximum(max);
    setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::setMinimum(double min)
{
    QDoubleSpinBox::setMinimum(min);
    setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::flicker()
{
    if (qFuzzyIsNull(value()))
        for (int i = 0, t = 0; i < 3; ++i) {
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, this, &DoubleSpinBox::red);
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, this, &DoubleSpinBox::normal);
        }
}

void DoubleSpinBox::red() { setStyleSheet("QWidget{ background-color: red; }"); }

void DoubleSpinBox::normal() { setStyleSheet(""); }

void DoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    //    if (event->key() == Qt::Key_Backspace) {
    //        QString text(lineEdit()->text());
    //        int start = lineEdit()->selectionStart();
    //        text.remove(--start, 1);
    //        lineEdit()->setText(text);
    //        lineEdit()->setSelection(start, 100);
    //        return;
    //    }
    if (event->text() == '.' || event->text() == ',') {
        QKeyEvent ke(event->type(), Qt::Key_Comma, event->modifiers(), QLocale().decimalPoint());
        QDoubleSpinBox::keyPressEvent(&ke);
    } else
        QDoubleSpinBox::keyPressEvent(event);
    //    lineEdit()->setSelection(lineEdit()->cursorPosition(), 100);
}

bool DoubleSpinBox::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        lineEdit()->setSelection(0, lineEdit()->text().length() - suffix().length()); //->selectAll();
    return QDoubleSpinBox::eventFilter(watched, event);
}
