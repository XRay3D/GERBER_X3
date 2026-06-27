/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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
                {      u"E"_s,       u"Math.E"_s}, // const
                {   u"LN10"_s,    u"Math.LN10"_s}, // const
                {    u"LN2"_s,     u"Math.LN2"_s}, // const
                { u"LOG10E"_s,  u"Math.LOG10E"_s}, // const
                {  u"LOG2E"_s,   u"Math.LOG2E"_s}, // const
                {     u"PI"_s,      u"Math.PI"_s}, // const
                {u"SQRT1_2"_s, u"Math.SQRT1_2"_s}, // const
                {  u"SQRT2"_s,   u"Math.SQRT2"_s}, // const
                {    u"abs"_s,     u"Math.abs"_s}, // func
                {   u"acos"_s,    u"Math.acos"_s}, // func
                {  u"acosh"_s,   u"Math.acosh"_s}, // func
                {   u"asin"_s,    u"Math.asin"_s}, // func
                {  u"asinh"_s,   u"Math.asinh"_s}, // func
                {   u"atan"_s,    u"Math.atan"_s}, // func
                {  u"atan2"_s,   u"Math.atan2"_s}, // func
                {  u"atanh"_s,   u"Math.atanh"_s}, // func
                {   u"cbrt"_s,    u"Math.cbrt"_s}, // func
                {   u"ceil"_s,    u"Math.ceil"_s}, // func
                {  u"clz32"_s,   u"Math.clz32"_s}, // func
                {    u"cos"_s,     u"Math.cos"_s}, // func
                {   u"cosh"_s,    u"Math.cosh"_s}, // func
                {    u"exp"_s,     u"Math.exp"_s}, // func
                {  u"expm1"_s,   u"Math.expm1"_s}, // func
                {  u"floor"_s,   u"Math.floor"_s}, // func
                { u"fround"_s,  u"Math.fround"_s}, // func
                {  u"hypot"_s,   u"Math.hypot"_s}, // func
                {   u"imul"_s,    u"Math.imul"_s}, // func
                {    u"log"_s,     u"Math.log"_s}, // func
                {  u"log10"_s,   u"Math.log10"_s}, // func
                {  u"log1p"_s,   u"Math.log1p"_s}, // func
                {   u"log2"_s,    u"Math.log2"_s}, // func
                {    u"max"_s,     u"Math.max"_s}, // func
                {    u"min"_s,     u"Math.min"_s}, // func
                {    u"pow"_s,     u"Math.pow"_s}, // func
                { u"random"_s,  u"Math.random"_s}, // func
                {  u"round"_s,   u"Math.round"_s}, // func
                {   u"sign"_s,    u"Math.sign"_s}, // func
                {    u"sin"_s,     u"Math.sin"_s}, // func
                {   u"sinh"_s,    u"Math.sinh"_s}, // func
                {   u"sqrt"_s,    u"Math.sqrt"_s}, // func
                {    u"tan"_s,     u"Math.tan"_s}, // func
                {   u"tanh"_s,    u"Math.tanh"_s}, // func
                {  u"trunc"_s,   u"Math.trunc"_s}, // func
            };
            for(auto&& [from, to]: array)
                str.replace(from, to, Qt::CaseInsensitive);
        } else {
            static QRegularExpression re{
                u"(E|LN10|LN2|LOG10E|LOG2E|PI|SQRT1_2|SQRT2|"_s
                u"abs|acos|acosh|asin|asinh|atan|atan2|atanh|"_s
                u"cbrt|ceil|clz32|cos|cosh|exp|expm1|floor|"_s
                u"fround|hypot|imul|log|log10|log1p|log2|max|"_s
                u"min|pow|random|round|sign|sin|sinh|sqrt|tan|tanh|trunc)"_s,
                QRegularExpression::CaseInsensitiveOption};
            str.replace(re, uR"(Math.\1)"_s);
        }
        qWarning() << timer.nsecsElapsed() / 1000 << u"us"_s << str;
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

void DoubleSpinBox::red() { setStyleSheet(u"QWidget{ background-color: red; }"_s); }

void DoubleSpinBox::normal() { setStyleSheet({}); }

void DoubleSpinBox::updateToolTip() {
    setToolTip(QString(QObject::tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::keyPressEvent(QKeyEvent* event) {
    static const auto decimalPoint = QLocale().decimalPoint();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if(event->modifiers().testFlags(Qt::ShiftModifier | Qt::ControlModifier)) setSingleStep(0.01);
#else
    if(event->modifiers().testFlag(Qt::ControlModifier)
        || event->modifiers().testFlag(Qt::ShiftModifier)) setSingleStep(0.01);
#endif
    if(event->text() == u'.' || event->text() == u',') {
        QKeyEvent ke{event->type(), decimalPoint == u'.' ? Qt::Key_Period : Qt::Key_Comma, event->modifiers(), decimalPoint};
        QDoubleSpinBox::keyPressEvent(&ke);
        event->accept();
    } else
        QDoubleSpinBox::keyPressEvent(event);
}

void DoubleSpinBox::keyReleaseEvent(QKeyEvent* event) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if(event->modifiers().testAnyFlags(Qt::NoModifier)) setSingleStep(1.0);
#else
    if(!event->modifiers().testFlag(Qt::NoModifier)) setSingleStep(1.0);
#endif
    QDoubleSpinBox::keyPressEvent(event);
}

double DoubleSpinBox::valueFromText(const QString& /*text*/) const {

    qWarning() << __FUNCTION__ << (bool)value_ << value_.value_or(value()) << value();
    return value_.value_or(value());
}

// QString DoubleSpinBox::textFromValue(double value) const {
// return QDoubleSpinBox::textFromValue(value);
// }

QValidator::State DoubleSpinBox::validate(QString& input, int& pos) const {
    static const std::set set{u'.', u',', u'/', u'*', u'-', u'+'};
    // Invalid      0 Строка явно недействительна.
    // Intermediate 1 Строка является вероятным промежуточным значением.
    // Acceptable   2 Строка приемлема в качестве конечного результата; то есть это действительно.
    auto fixMath = [this](QString str) {
        QElapsedTimer timer;
        timer.start();
        const QString sfx = suffix();
        if(str.endsWith(sfx)) str.chop(sfx.size());
        if constexpr(0) {
            static const std::pair<QString, QString> array[]{
                {u"E"_s,       u"Math.E"_s      }, // const
                {u"LN10"_s,    u"Math.LN10"_s   }, // const
                {u"LN2"_s,     u"Math.LN2"_s    }, // const
                {u"LOG10E"_s,  u"Math.LOG10E"_s }, // const
                {u"LOG2E"_s,   u"Math.LOG2E"_s  }, // const
                {u"PI"_s,      u"Math.PI"_s     }, // const
                {u"SQRT1_2"_s, u"Math.SQRT1_2"_s}, // const
                {u"SQRT2"_s,   u"Math.SQRT2"_s  }, // const
                {u"abs"_s,     u"Math.abs"_s    }, // func
                {u"acos"_s,    u"Math.acos"_s   }, // func
                {u"acosh"_s,   u"Math.acosh"_s  }, // func
                {u"asin"_s,    u"Math.asin"_s   }, // func
                {u"asinh"_s,   u"Math.asinh"_s  }, // func
                {u"atan"_s,    u"Math.atan"_s   }, // func
                {u"atan2"_s,   u"Math.atan2"_s  }, // func
                {u"atanh"_s,   u"Math.atanh"_s  }, // func
                {u"cbrt"_s,    u"Math.cbrt"_s   }, // func
                {u"ceil"_s,    u"Math.ceil"_s   }, // func
                {u"clz32"_s,   u"Math.clz32"_s  }, // func
                {u"cos"_s,     u"Math.cos"_s    }, // func
                {u"cosh"_s,    u"Math.cosh"_s   }, // func
                {u"exp"_s,     u"Math.exp"_s    }, // func
                {u"expm1"_s,   u"Math.expm1"_s  }, // func
                {u"floor"_s,   u"Math.floor"_s  }, // func
                {u"fround"_s,  u"Math.fround"_s }, // func
                {u"hypot"_s,   u"Math.hypot"_s  }, // func
                {u"imul"_s,    u"Math.imul"_s   }, // func
                {u"log"_s,     u"Math.log"_s    }, // func
                {u"log10"_s,   u"Math.log10"_s  }, // func
                {u"log1p"_s,   u"Math.log1p"_s  }, // func
                {u"log2"_s,    u"Math.log2"_s   }, // func
                {u"max"_s,     u"Math.max"_s    }, // func
                {u"min"_s,     u"Math.min"_s    }, // func
                {u"pow"_s,     u"Math.pow"_s    }, // func
                {u"random"_s,  u"Math.random"_s }, // func
                {u"round"_s,   u"Math.round"_s  }, // func
                {u"sign"_s,    u"Math.sign"_s   }, // func
                {u"sin"_s,     u"Math.sin"_s    }, // func
                {u"sinh"_s,    u"Math.sinh"_s   }, // func
                {u"sqrt"_s,    u"Math.sqrt"_s   }, // func
                {u"tan"_s,     u"Math.tan"_s    }, // func
                {u"tanh"_s,    u"Math.tanh"_s   }, // func
                {u"trunc"_s,   u"Math.trunc"_s  }, // func
            };
            for(auto&& [from, to]: array)
                str.replace(from, to, Qt::CaseInsensitive);
        } else {
            static QRegularExpression re{
                u"(E|LN10|LN2|LOG10E|LOG2E|PI|SQRT1_2|SQRT2|"_s
                u"abs|acos|acosh|asin|asinh|atan|atan2|atanh|"_s
                u"cbrt|ceil|clz32|cos|cosh|exp|expm1|floor|"_s
                u"fround|hypot|imul|log|log10|log1p|log2|max|"_s
                u"min|pow|random|round|sign|sin|sinh|sqrt|tan|tanh|trunc)"_s,
                QRegularExpression::CaseInsensitiveOption};
            str.replace(re, uR"(Math.\1)"_s);
        }
        str.replace(u',', u'.');
        qWarning() << __FUNCTION__ << timer.nsecsElapsed() / 1000 << u"us"_s << str;
        return str;
    };
    if(input.size() == 0) return QValidator::Intermediate;
    if(input.count(u'(') != input.count(u')')) return QValidator::Intermediate;
    if(pos) { // skip duplicates
        static QRegularExpression re{
            uR"(.*[\.\,\/\*\-\+]{2,}.*)"_s,
            QRegularExpression::CaseInsensitiveOption};
        if(re.match(input).hasMatch()) return QValidator::Invalid;

        // auto ch = input[pos - 1];
        // if(pos > 1 && set.contains(ch) && input[pos - 2] == ch)
        // return QValidator::Invalid;
        // if(pos < input.size() && set.contains(ch) && input[pos] == ch)
        // return QValidator::Invalid;
    }
    auto val = jsEngine->evaluate(fixMath(input));
    auto error = val.errorType();
    error ? value_ = std::nullopt : value_ = val.toNumber();
    qWarning() << __FUNCTION__ << error << val.toString();
    // if(!error) str = val.toString();
    return error ? QValidator::Intermediate : QValidator::Acceptable;
}
