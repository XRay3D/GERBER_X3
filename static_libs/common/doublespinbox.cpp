// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "doublespinbox.h"
#include "mathparser.h"
#include <QDebug>

DoubleSpinBox::DoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent) {
    lineEdit()->installEventFilter(this);
    lineEdit()->setValidator(nullptr);
    setToolTipDuration(0);
}

void DoubleSpinBox::setRange(double min, double max) {
    QDoubleSpinBox::setRange(min, max);
    setToolTip(QString(QObject::tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::setMaximum(double max) {
    QDoubleSpinBox::setMaximum(max);
    setToolTip(QString(QObject::tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::setMinimum(double min) {
    QDoubleSpinBox::setMinimum(min);
    setToolTip(QString(QObject::tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::flicker() {
    if (qFuzzyIsNull(value()))
        for (int i = 0, t = 0; i < 3; ++i) {
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, this, &DoubleSpinBox::red);
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, this, &DoubleSpinBox::normal);
        }
}

bool DoubleSpinBox::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease)
        lineEdit()->setSelection(0, lineEdit()->text().length() - suffix().length()); //->selectAll();
    return QDoubleSpinBox::eventFilter(watched, event);
}

void DoubleSpinBox::red() { setStyleSheet("QWidget{ background-color: red; }"); }

void DoubleSpinBox::normal() { setStyleSheet(""); }

void DoubleSpinBox::keyPressEvent(QKeyEvent* event) {
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

double DoubleSpinBox::valueFromText(const QString& text) const {
    return QDoubleSpinBox::valueFromText(text);
    qDebug() << __FUNCTION__ << text;
    str = text.mid(0, text.size() - suffix().size());
    try {
        if (str.size())
            return MathParser(nullptr).parse(str);
    } catch (...) { }
    return 0;
}

QString DoubleSpinBox::textFromValue(double value) const {
    return QDoubleSpinBox::textFromValue(value);
    qDebug(__FUNCTION__);
    return str;
}

QValidator::State DoubleSpinBox::validate(QString& input, int& pos) const {
    return QDoubleSpinBox::validate(input, pos);
    qDebug(__FUNCTION__);
    return QValidator::Acceptable;
}

void DoubleSpinBox::fixup(QString& input) const {
    QDoubleSpinBox::fixup(input);
    qDebug(__FUNCTION__);
}
