/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "doublespinbox.h"
// #include "mathparser.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTimer>
#include <QtQml/QJSEngine>
#include <cmath>
#include <numbers>
#include <set>
#if 0
struct Validator : QValidator {
    Validator(DoubleSpinBox* dsbx)
        : QValidator{dsbx}
        , dsbx{dsbx}
        , jsEngine{new QJSEngine{dsbx}} { }
    DoubleSpinBox* const dsbx;
    QJSEngine* const jsEngine;

    // QValidator interface
    State validate(QString& str, int& pos) const override {
        static const std::set<QChar> set{'.', ',', '/', '*', '-', '+'};
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
        auto error = val.errorType();
        error ? dsbx->value_ = std::nullopt : dsbx->value_ = val.toNumber();
        qWarning() << __FUNCTION__ << error << val.toString();
        // if(!error) str = val.toString();
        return error ? Intermediate : Acceptable;
    }
};
#endif

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
    : QDoubleSpinBox{parent}
    , jsEngine{new QJSEngine{this}} {
    lineEdit()->installEventFilter(this);
    lineEdit()->setValidator(nullptr);
    installEventFilter(this);
    setToolTipDuration(0);
}

void DoubleSpinBox::setRange(double min, double max) {
    QDoubleSpinBox::setRange(min, max), updateToolTip();
}

void DoubleSpinBox::setMaximum(double max) {
    QDoubleSpinBox::setMaximum(max), updateToolTip();
}

void DoubleSpinBox::setMinimum(double min) {
    QDoubleSpinBox::setMinimum(min), updateToolTip();
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
    } else if(this == watched && (event->type() == QEvent::KeyRelease || event->type() == QEvent::KeyPress)) {
        auto key = static_cast<QKeyEvent*>(event)->key();
        if((key == Qt::Key_Delete /*|| key == Qt::Key_Backspace*/)
            && lineEdit()->cursorPosition() == (lineEdit()->text().length() - suffix().length())) {
            qWarning() << __FUNCTION__ << watched << static_cast<QKeyEvent*>(event)->key();
            return true;
        }
    }
    return QDoubleSpinBox::eventFilter(watched, event);
}

void DoubleSpinBox::red() { setStyleSheet("QWidget{ background-color: red; }"); }

void DoubleSpinBox::normal() { setStyleSheet(""); }

void DoubleSpinBox::updateToolTip() {
    setToolTip(QString(QObject::tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::keyPressEvent(QKeyEvent* event) {
    static const auto decimalPoint = QLocale().decimalPoint();
#if(QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if(event->modifiers().testFlags(Qt::ShiftModifier | Qt::ControlModifier)) setSingleStep(0.01);
#else
    if(event->modifiers().testFlag(Qt::ControlModifier)
        || event->modifiers().testFlag(Qt::ShiftModifier)) setSingleStep(0.01);
#endif
    if(event->text() == '.' || event->text() == ',') {
        QKeyEvent ke{event->type(), decimalPoint == '.' ? Qt::Key_Period : Qt::Key_Comma, event->modifiers(), decimalPoint};
        QDoubleSpinBox::keyPressEvent(&ke);
        event->accept();
    } else
        QDoubleSpinBox::keyPressEvent(event);
}

void DoubleSpinBox::keyReleaseEvent(QKeyEvent* event) {
#if(QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if(event->modifiers().testAnyFlags(Qt::NoModifier)) setSingleStep(1.0);
#else
    if(!event->modifiers().testFlag(Qt::NoModifier)) setSingleStep(1.0);
#endif
    QDoubleSpinBox::keyPressEvent(event);
}

double DoubleSpinBox::valueFromText(const QString& text) const {

    qWarning() << __FUNCTION__ << (bool)value_ << value_.value_or(value()) << value();
    return value_.value_or(value());
}

// QString DoubleSpinBox::textFromValue(double value) const {
//     return QDoubleSpinBox::textFromValue(value);
// }

QValidator::State DoubleSpinBox::validate(QString& input, int& pos) const {

    static const std::set<QChar> set{'.', ',', '/', '*', '-', '+'};
    // Invalid      0 Строка явно недействительна.
    // Intermediate 1 Строка является вероятным промежуточным значением.
    // Acceptable   2 Строка приемлема в качестве конечного результата; то есть это действительно.
    auto fixMath = [this](QString& str) {
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
        str.replace(',', '.');
        qWarning() << __FUNCTION__ << timer.nsecsElapsed() / 1000 << "us" << str;
        return str.mid(0, str.size() - suffix().size());
    };
    if(input.size() == 0) return QValidator::Intermediate;
    if(input.count('(') != input.count(')')) return QValidator::Intermediate;
    if(pos) { // skip duplicates
        static QRegularExpression re{
            R"(.*[\.\,\/\*\-\+]{2,}.*)",
            QRegularExpression::CaseInsensitiveOption};
        if(re.match(input).hasMatch()) return QValidator::Invalid;

        // auto ch = input[pos - 1];
        // if(pos > 1 && set.contains(ch) && input[pos - 2] == ch)
        //     return QValidator::Invalid;
        // if(pos < input.size() && set.contains(ch) && input[pos] == ch)
        //     return QValidator::Invalid;
    }
    auto val = jsEngine->evaluate(fixMath(input));
    auto error = val.errorType();
    error ? value_ = std::nullopt : value_ = val.toNumber();
    qWarning() << __FUNCTION__ << error << val.toString();
    // if(!error) str = val.toString();
    return error ? QValidator::Intermediate : QValidator::Acceptable;
}
