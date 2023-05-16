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
#include <numbers>
#include <cmath>

DoubleSpinBox::DoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent) {
    lineEdit()->installEventFilter(this);
    installEventFilter(this);
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
    if(qFuzzyIsNull(value()))
        for(int i{}; i < 6;) {
            QTimer::singleShot(++i * 150, Qt::CoarseTimer, this, &DoubleSpinBox::red);
            QTimer::singleShot(++i * 150, Qt::CoarseTimer, this, &DoubleSpinBox::normal);
        }
}

bool DoubleSpinBox::eventFilter(QObject* watched, QEvent* event) {
    if(lineEdit() == watched && event->type() == QEvent::MouseButtonRelease) {
        lineEdit()->setSelection(0, lineEdit()->text().length() - suffix().length());
    } else if(this == watched
        && (event->type() == QEvent::KeyRelease || event->type() == QEvent::KeyPress)) {
        auto key = static_cast<QKeyEvent*>(event)->key();
        if((key == Qt::Key_Delete /*|| key == Qt::Key_Backspace*/)
            && lineEdit()->cursorPosition() == (lineEdit()->text().length() - suffix().length())) {
            qDebug() << watched << static_cast<QKeyEvent*>(event)->key();
            return true;
        }
    }
    return QDoubleSpinBox::eventFilter(watched, event);
}

void DoubleSpinBox::red() { setStyleSheet("QWidget{ background-color: red; }"); }

void DoubleSpinBox::normal() { setStyleSheet(""); }

void DoubleSpinBox::keyPressEvent(QKeyEvent* event) {
    static const auto decimalPoint = QLocale().decimalPoint();

    if(event->text() == '.' || event->text() == ',') {
        QKeyEvent ke{event->type(), decimalPoint == '.' ? Qt::Key_Period : Qt::Key_Comma, event->modifiers(), decimalPoint};
        QDoubleSpinBox::keyPressEvent(&ke);
        event->accept();
    } else
        QDoubleSpinBox::keyPressEvent(event);
}

double DoubleSpinBox::valueFromText(const QString& text) const {
    // return QDoubleSpinBox::valueFromText(text);
    str = text.mid(0, text.size() - suffix().size());
    double val{};
    try {
        static VarMap varmap{
            {         "e",          std::numbers::e},
            {    "egamma",     std::numbers::egamma},
            {    "inv_pi",     std::numbers::inv_pi},
            { "inv_sqrt3",  std::numbers::inv_sqrt3},
            {"inv_sqrtpi", std::numbers::inv_sqrtpi},
            {      "ln10",       std::numbers::ln10},
            {       "ln2",        std::numbers::ln2},
            {    "log10e",     std::numbers::log10e},
            {     "log2e",      std::numbers::log2e},
            {       "phi",        std::numbers::phi},
            {        "pi",         std::numbers::pi},
            {     "sqrt2",      std::numbers::sqrt2},
            {     "sqrt3",      std::numbers::sqrt3},
        };
        if(str.size())
            val = MathParser(&varmap).parse(str.replace(',', '.'));
        if(std::isnan(val) || std::isinf(val))
            return value();
    } catch(...) { } //
    return val;
}

QString DoubleSpinBox::textFromValue(double value) const {
    return QDoubleSpinBox::textFromValue(value);
}

QValidator::State DoubleSpinBox::validate(QString& input, int& pos) const {
    // qDebug() << input << pos;
    return QValidator::Acceptable;
}

// void DoubleSpinBox::fixup(QString& input) const {
//     qDebug() << input;
//     QDoubleSpinBox::fixup(input);
//     qDebug() << input;
// }
