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
#include <cmath>
#include <numbers>

#if 0
struct Validator : QValidator {
    using QValidator::QValidator;
    Validator(QJSEngine* jsEngine, QObject* parent = nullptr)
        : QValidator{parent}
        , jsEngine{jsEngine} { }
    QJSEngine* const jsEngine;
    static auto fixMath(QString str) {
        QElapsedTimer timer;
        timer.start();
        if constexpr(0) {
            static const std::pair<QString, QString> array[]{
                {      "E",       "Math.E"}, // const
                {   "LN10",    "Math.LN10"}, // const
                {    "LN2",     "Math.LN2"}, // const
                { "LOG10E",  "Math.LOG10E"}, // const
                {  "LOG2E",   "Math.LOG2E"}, // const
                {     "PI",      "Math.PI"}, // const
                {"SQRT1_2", "Math.SQRT1_2"}, // const
                {  "SQRT2",   "Math.SQRT2"}, // const
                {    "abs",     "Math.abs"}, // func
                {   "acos",    "Math.acos"}, // func
                {  "acosh",   "Math.acosh"}, // func
                {   "asin",    "Math.asin"}, // func
                {  "asinh",   "Math.asinh"}, // func
                {   "atan",    "Math.atan"}, // func
                {  "atan2",   "Math.atan2"}, // func
                {  "atanh",   "Math.atanh"}, // func
                {   "cbrt",    "Math.cbrt"}, // func
                {   "ceil",    "Math.ceil"}, // func
                {  "clz32",   "Math.clz32"}, // func
                {    "cos",     "Math.cos"}, // func
                {   "cosh",    "Math.cosh"}, // func
                {    "exp",     "Math.exp"}, // func
                {  "expm1",   "Math.expm1"}, // func
                {  "floor",   "Math.floor"}, // func
                { "fround",  "Math.fround"}, // func
                {  "hypot",   "Math.hypot"}, // func
                {   "imul",    "Math.imul"}, // func
                {    "log",     "Math.log"}, // func
                {  "log10",   "Math.log10"}, // func
                {  "log1p",   "Math.log1p"}, // func
                {   "log2",    "Math.log2"}, // func
                {    "max",     "Math.max"}, // func
                {    "min",     "Math.min"}, // func
                {    "pow",     "Math.pow"}, // func
                { "random",  "Math.random"}, // func
                {  "round",   "Math.round"}, // func
                {   "sign",    "Math.sign"}, // func
                {    "sin",     "Math.sin"}, // func
                {   "sinh",    "Math.sinh"}, // func
                {   "sqrt",    "Math.sqrt"}, // func
                {    "tan",     "Math.tan"}, // func
                {   "tanh",    "Math.tanh"}, // func
                {  "trunc",   "Math.trunc"}, // func
            };
            for(auto&& [from, to]: array)
                str.replace(from, to, Qt::CaseInsensitive);
        } else {
            static QRegularExpression re{
                "(E|LN10|LN2|LOG10E|LOG2E|PI|SQRT1_2|SQRT2|"
                "abs|acos|acosh|asin|asinh|atan|atan2|atanh|"
                "cbrt|ceil|clz32|cos|cosh|exp|expm1|floor|"
                "fround|hypot|imul|log|log10|log1p|log2|max|"
                "min|pow|random|round|sign|sin|sinh|sqrt|tan|tanh|trunc)",
                QRegularExpression::CaseInsensitiveOption};
            str.replace(re, R"(Math.\1)");
        }
        qWarning() << timer.nsecsElapsed() / 1000 << "us" << str;
        return str;
    }
    const std::set<QChar> set{'.', '/', '*', '-', '+'};
    // QValidator interface
    State validate(QString& str, int& pos) const override {
        // Invalid      0 Строка явно недействительна.
        // Intermediate 1 Строка является вероятным промежуточным значением.
        // Acceptable   2 Строка приемлема в качестве конечного результата; то есть это действительно.
        if(str.size() == 0) return Intermediate;
        if(str.count('(') != str.count(')')) return Intermediate;
        if(pos) { // skip duplicates
            auto ch = str[pos - 1];
            if(pos > 1 && set.contains(ch) && str[pos - 2] == ch)
                return Invalid;
            if(pos < str.size() && set.contains(ch) && str[pos] == ch)
                return Invalid;
        }
        auto val = jsEngine->evaluate(fixMath(str));
        if(val.errorType())
            qDebug() << val.toString();
        return val.errorType() ? Intermediate : Acceptable;
    }
};
#endif

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
        // static VarMap varmap{
        //     {         "e",          std::numbers::e},
        //     {    "egamma",     std::numbers::egamma},
        //     {    "inv_pi",     std::numbers::inv_pi},
        //     { "inv_sqrt3",  std::numbers::inv_sqrt3},
        //     {"inv_sqrtpi", std::numbers::inv_sqrtpi},
        //     {      "ln10",       std::numbers::ln10},
        //     {       "ln2",        std::numbers::ln2},
        //     {    "log10e",     std::numbers::log10e},
        //     {     "log2e",      std::numbers::log2e},
        //     {       "phi",        std::numbers::phi},
        //     {        "pi",         std::numbers::pi},
        //     {     "sqrt2",      std::numbers::sqrt2},
        //     {     "sqrt3",      std::numbers::sqrt3},
        // };
        // if(str.size())
        //     val = MathParser(&varmap).parse(str.replace(',', '.'));

        static MathParser parser{
            {
             {"e", std::numbers::e},
             {"egamma", std::numbers::egamma},
             {"inv_pi", std::numbers::inv_pi},
             {"inv_sqrt3", std::numbers::inv_sqrt3},
             {"inv_sqrtpi", std::numbers::inv_sqrtpi},
             {"ln10", std::numbers::ln10},
             {"ln2", std::numbers::ln2},
             {"log10e", std::numbers::log10e},
             {"log2e", std::numbers::log2e},
             {"phi", std::numbers::phi},
             {"pi", std::numbers::pi},
             {"sqrt2", std::numbers::sqrt2},
             {"sqrt3", std::numbers::sqrt3},
             }
        };
        if(str.size())
            val = parser.parse(str.replace(',', '.').toStdString());

        if(std::isnan(val) || std::isinf(val))
            return value();
    } catch(...) { } //
    return val;
}

QString DoubleSpinBox::textFromValue(double value) const {
    return QDoubleSpinBox::textFromValue(value);
}

QValidator::State DoubleSpinBox::validate(QString& input, int& pos) const {
    // bool ok{};
    // QString{input}.toDouble(&ok);
    // qInfo() << input << pos;
    // return ok ? QValidator::Intermediate : QValidator::Acceptable;
    return QValidator::Acceptable;
}

// void DoubleSpinBox::fixup(QString& input) const {
//     qDebug() << input;
//     QDoubleSpinBox::fixup(input);
//     qDebug() << input;
// }
