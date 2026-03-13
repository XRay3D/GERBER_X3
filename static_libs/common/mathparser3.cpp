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
#include "mathparser3.h"

#include <QDebug>
// #include <QStringBuilder>
#include <cmath>
#include <functional>

using namespace Qt::Literals;

MathParser3::MathParser3(VarMap* variables)
    : variables{variables} { }

double MathParser3::getVariable(QStringView variableName) {
    if(!variables || !variables->contains(variableName.toString())) {
        qWarning() << u"Error: Try get unexists variable '" + variableName.toString() + u"'";
        return 0.0;
    }
    return variables->at(variableName.toString());
}

double MathParser3::parse(const QString& s) {
    Result result;

    try {
        result = plusMinus(s);
        if(result.rest.size())
            qWarning() << u"Error: can't full parse\"" + s + u"\"rest: " << result.rest;
    } catch(const QString& str) {
        qWarning() << str;
    }
    return result.acc;
}

double MathParser3::parse(QStringView s) {
    Result result;

    try {
        result = plusMinus(s);
        if(result.rest.size())
            qWarning() << u"Error: can't full parse\"" + s.toString() + u"\"rest: " << result.rest;
    } catch(const QString& str) {
        qWarning() << str;
    }
    return result.acc;
}

MathParser3::Result MathParser3::plusMinus(QStringView s) {
    Result current = mulDiv(s);
    double acc = current.acc;

    while(current.rest.length() > 0) {
        if(!(current.rest.at(0) == u'+' || current.rest.at(0) == u'-'))
            break;

        QChar sign = current.rest.at(0);
        QStringView next = current.rest.mid(1);
        if(next.empty())
            throw u"next.empty()"_s;
        current = mulDiv(next);
        if(sign == u'+')
            acc += current.acc;
        else
            acc -= current.acc;
    }
    return Result{acc, current.rest};
}

MathParser3::Result MathParser3::bracket(QStringView s) {
    QChar zeroChar = s.at(0);
    if(zeroChar == u'(') {
        Result r = plusMinus(s.mid(1));
        if(!r.rest.isEmpty() && r.rest.at(0) == u')')
            r.rest = r.rest.mid(1);
        else
            qWarning() << u"Error: not close bracket";
        return r;
    }
    return functionVariable(s);
}

MathParser3::Result MathParser3::functionVariable(QStringView s) {
    QStringView f;
    int i{};
    int sign = +1;
    if(s.startsWith(u'-')) {
        sign = -1;
        s = s.mid(1); // s.remove(0, 1);
    }
    // ищем название функции или переменной
    // имя обязательно должна начинаться с буквы
    while(i < s.length() && ((s.at(i).isLetter() || s.at(i) == u'$') || (s.at(i).isDigit() && i > 0))) {
        // while (i < s.length() && (s.at(i).isLetter() || (s.at(i).isDigit() && i > 0))) {
        // f += s.at(i);
        i++;
    }
    f = s.mid(0, i);

    if(!f.isEmpty()) {                          // если что-нибудь нашли
        if(s.length() > i && s.at(i) == u'(') { // и следующий символ скобка значит - это функция
            Result r = bracket(s.mid(f.length()));
            return processFunction(f, r);
        } else // иначе - это переменная
            return Result{getVariable(f) * sign, s.mid(f.length())};
    }
    return num(s);
}

MathParser3::Result MathParser3::mulDiv(QStringView s) {
    if(s.empty())
        throw u"mulDiv s.empty()"_s;
    Result current = bracket(s);

    double acc = current.acc;
    while(true) {
        if(current.rest.length() == 0)
            return current;

        QChar sign = current.rest.at(0);
        if((sign != u'*' && sign != u'/'))
            return current;

        QStringView next = current.rest.mid(1);
        if(next.empty())
            throw u"next.empty()"_s;
        Result right = bracket(next);

        if(sign == u'*')
            acc *= right.acc;
        else
            acc /= right.acc;

        current = Result{acc, right.rest};
    }
}

MathParser3::Result MathParser3::num(QStringView s) {
    int i{};
    int dot_cnt{};
    bool negative{};
    // число также может начинаться с минуса
    if(s.at(0) == u'-') {
        negative = true;
        s = s.mid(1);
    }
    // разрешаем только цифры и точку
    while(i < s.length() && (s.at(i).isDigit() || s.at(i) == u'.')) {
        // но также проверям, что в числе может быть только одна точка!
        if(s.at(i) == u'.' && ++dot_cnt > 1)
            throw u"not valid number '%1'"_s.arg(s.mid(0, i + 1));

        i++;
    }
    if(i == 0) // что-либо похожее на число мы не нашли
        throw u"can't get valid number in '%1'"_s.arg(s);

    double dPart = s.mid(0, i).toDouble();
    if(negative)
        dPart = -dPart;

    return Result{dPart, s.mid(i)};
}

MathParser3::Result MathParser3::processFunction(QStringView func, Result r) {

    using F = double (*)(double);
    static std::unordered_map<QStringView, F> funcMap{
        { u"cos",  [](double val) { return cos(val); }},
        { u"sin",  [](double val) { return sin(val); }},
        {u"sqrt", [](double val) { return sqrt(val); }},
        { u"tan",  [](double val) { return tan(val); }},
    };

    if(funcMap.contains(func))
        return Result{funcMap[func](r.acc), r.rest};
    else
        qWarning() << u"function '" + func.toString() + u"' is not defined";

    //    enum class Func {
    //        sin,
    //        cos,
    //        tan
    //    };
    //    switch (Func(u"sin,cos,tan"_s.split(u',').indexOf(func))) {
    //    case Func::sin:
    //        return Result {sin(r.acc), r.rest};
    //    case Func::cos:
    //        return Result {cos(r.acc), r.rest};
    //    case Func::tan:
    //        return Result {tan(r.acc), r.rest};
    //    default:
    //        qWarning() << "function '" + func + "' is not defined";
    //        break;
    //    }
    return r;
}
