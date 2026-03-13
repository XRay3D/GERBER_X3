///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  XXXXX XX, 2025                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2025                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
#include "mathparser1.h"

/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */
// #include "utils.h"
#include <QDebug>
#include <QStringBuilder>
#include <boost/stacktrace.hpp>
#include <charconv>
#include <cmath>
#include <sstream>

using namespace Qt::Literals;

template <> struct QConcatenable<sv> : private QAbstractConcatenable {
    using type = sv;
    using ConvertTo = QString;
    static constexpr bool ExactSize = true;
    static int size(sv a) { return a.size(); }
    static inline void QT_ASCII_CAST_WARN appendTo(sv a, QChar*& out) {
        for(auto c: a) *out++ = c;
    }
};

MathParser1::MathParser1(VarMap* variables)
    : variables{variables} {
}

double MathParser1::getVariable(const QString& varName) {

    if(!variables || !variables->contains(varName)) {
        qWarning() << u"Error: Try get unexists variable '" + varName + u"'";
        return 0.0;
    }
    return variables->at(varName);
}

double MathParser1::parse(const QString& s) {

    Result result;
    try {
        result = plusMinus(sv{reinterpret_cast<const char16_t*>(s.data()), size_t(s.size())});
        if(result.rest.size()) {
            // std::stringstream ss;
            // ss << boost::stacktrace::stacktrace();
            // qWarning() << QString::fromStdString(ss.str());
            qWarning() << "Error: can't full parse'" << s << "'rest: " << result.rest.data();
        }
    } catch(const sv& str) {
        qWarning() << str.data();
    }
    return result.acc;
}

MathParser1::Result MathParser1::plusMinus(sv s) {
    Result current = mulDiv(s);
    double acc = current.acc;

    while(current.rest.length() > 0) {
        if(!(current.rest.at(0) == u'+' || current.rest.at(0) == u'-'))
            break;

        QChar sign = current.rest.at(0);
        sv next = current.rest.substr(1);

        current = mulDiv(next);
        if(sign == u'+')
            acc += current.acc;
        else
            acc -= current.acc;
    }
    return Result(acc, current.rest);
}

MathParser1::Result MathParser1::bracket(sv s) {

    QChar zeroChar = s.at(0);
    if(zeroChar == u'(') {
        Result r = plusMinus(s.substr(1));
        if(!r.rest.empty() && r.rest.at(0) == u')')
            r.rest = r.rest.substr(1);
        else
            qWarning() << "Error: not close bracket";
        return r;
    }
    return functionVariable(s);
}

MathParser1::Result MathParser1::functionVariable(sv s) {

    int sign{+1};
    if(s.starts_with('-')) {
        sign = -1;
        s = s.substr(1);
    }

    // ищем название функции или переменной имя обязательно должна начинаться с буквы
    sv f;
    size_t i{};

    while(i < s.length() && ((QChar(s.at(i)).isLetter() || s.at(i) == u'$') || (QChar(s.at(i)).isDigit() && i > 0))) {
        // while (i < s.length() && (QChar(s.at(i)).isLetter() || (QChar(s.at(i)).isDigit() && i > 0))) {
        // f += s.at(i);
        i++;
    }
    f = s.substr(0, i);

    if(!f.empty()) { // если что-нибудь нашли и следующий символ скобка значит - это функция
        if(s.length() > i && s.at(i) == u'(') {
            Result r = bracket(s.substr(f.length()));
            return processFunction(f, r);
        } else // иначе - это переменная
            return Result(getVariable(toString(f)) * sign, s.substr(f.length()));
    }
    return num(s);
}

MathParser1::Result MathParser1::mulDiv(sv s) {

    Result current = bracket(s);

    double acc = current.acc;
    while(true) {
        if(current.rest.length() == 0)
            return current;

        QChar sign = current.rest.at(0);
        if((sign != u'*' && sign != u'/'))
            return current;

        sv next = current.rest.substr(1);
        Result right = bracket(next);

        if(sign == u'*')
            acc *= right.acc;
        else
            acc /= right.acc;

        current = Result(acc, right.rest);
    }
}

MathParser1::Result MathParser1::num(sv s) {
    size_t i{};
    int dot_cnt{};
    bool negative{};
    // число также может начинаться с минуса
    if(s.at(0) == u'-') {
        negative = true;
        s = s.substr(1);
    }
    // разрешаем только цифры и точку
    while(i < s.length() && (iswdigit(s.at(i)) || s.at(i) == u'.')) {
        // но также проверям, что в числе может быть только одна точка!
        if(s.at(i) == u'.' && ++dot_cnt > 1)
            throw "not valid number '" % s.substr(0, i + 1) % "'";

        i++;
    }
    if(i == 0) // что-либо похожее на число мы не нашли
        throw "can't get valid number in '" % s % "'";

    double dPart = toDouble(s.substr(0, i));
    if(negative)
        dPart = -dPart;

    sv restPart = s.substr(i);

    return Result(dPart, restPart);
}

MathParser1::Result MathParser1::processFunction(sv func, const Result& r) {

    // if (func.starts_with("sin"))
    // return Result(sin(r.acc), r.rest);
    // if (func.starts_with("cos"))
    // return Result(cos(r.acc), r.rest);
    // if (func.starts_with("tan"))
    // return Result(tan(r.acc), r.rest);
    // return r;

    enum class Func {
        sin,
        cos,
        tan
    };
    switch(Func(u"sin,cos,tan"_s.split(u',').indexOf(toString(func)))) {
    case Func::sin:
        return Result(sin(r.acc), r.rest);
    case Func::cos:
        return Result(cos(r.acc), r.rest);
    case Func::tan:
        return Result(tan(r.acc), r.rest);
    default:
        qWarning() << "function '" << func.data() << "' is not defined";
        break;
    }
    return r;
}
