// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */
#include "mathparser.h"
#include <QDebug>
#include <charconv>
#include <cmath>

MathParser::MathParser(VarMap* variables)
    : variables(variables) {
}

double MathParser::getVariable(const QString& varName) {
    if (!variables->contains(varName)) {
        qWarning() << "Error: Try get unexists variable '" + varName + "'";
        return 0.0;
    }
    return variables->at(varName);
}

double MathParser::parse(const QString& s) {
    Result result;
    try {
        result = plusMinus(sv {(char16_t*)(s.data()), size_t(s.size())});
        if (!result.rest.size()) {
            qWarning() << "Error: can't full parse"
                       << "\"" << s << "\"";
            qWarning() << "rest: " << result.rest.data();
        }
    } catch (const sv& str) {
        qWarning() << str.data();
    }
    return result.acc;
}

Result MathParser::plusMinus(sv s) // throws Exception
{
    Result current = mulDiv(s);
    double acc = current.acc;

    while (current.rest.length() > 0) {
        if (!(current.rest.at(0) == '+' || current.rest.at(0) == '-'))
            break;

        QChar sign = current.rest.at(0);
        sv next = current.rest.substr(1);

        current = mulDiv(next);
        if (sign == '+')
            acc += current.acc;
        else
            acc -= current.acc;
    }
    return Result(acc, current.rest);
}

Result MathParser::bracket(sv s) // throws Exception
{
    QChar zeroChar = s.at(0);
    if (zeroChar == '(') {
        Result r = plusMinus(s.substr(1));
        if (!r.rest.empty() && r.rest.at(0) == ')')
            r.rest = r.rest.substr(1);
        else
            qWarning() << "Error: not close bracket";
        return r;
    }
    return functionVariable(s);
}

Result MathParser::functionVariable(sv s) // throws Exception
{
    int sign {1};
    if (s.starts_with('-')) {
        sign = -1;
        s = s.substr(1);
    }

    // ищем название функции или переменной
    // имя обязательно должна начинаться с буквы
    sv f;
    int i {};
    while (i < s.length() && ((iswalpha(s.at(i)) || s.at(i) == '$') || (iswdigit(s.at(i)) && i > 0))) {
        // while (i < s.length() && (s.at(i).isLetter() || (s.at(i).isDigit() && i > 0))) {
        // f += s.at(i);
        i++;
    }
    f = s.substr(0, --i);

    if (!f.empty()) {                           // если что-нибудь нашли
        if (s.length() > i && s.at(i) == '(') { // и следующий символ скобка значит - это функция
            Result r = bracket(s.substr(f.length()));
            return processFunction(f, r);
        } else // иначе - это переменная
            return Result(getVariable(toString(f)) * sign, s.substr(f.length()));
    }
    return num(s);
}

Result MathParser::mulDiv(sv s) // throws Exception
{
    Result current = bracket(s);

    double acc = current.acc;
    while (true) {
        if (current.rest.length() == 0)
            return current;

        QChar sign = current.rest.at(0);
        if ((sign != '*' && sign != '/'))
            return current;

        sv next = current.rest.substr(1);
        Result right = bracket(next);

        if (sign == '*')
            acc *= right.acc;
        else
            acc /= right.acc;

        current = Result(acc, right.rest);
    }
}

Result MathParser::num(sv s) // throws Exception
{
    int i = 0;
    int dot_cnt = 0;
    bool negative = false;
    // число также может начинаться с минуса
    if (s.at(0) == '-') {
        negative = true;
        s = s.substr(1);
    }
    // разрешаем только цифры и точку
    while (i < s.length() && (iswdigit(s.at(i)) || s.at(i) == '.')) {
        // но также проверям, что в числе может быть только одна точка!
        if (s.at(i) == '.' && ++dot_cnt > 1)
            throw "not valid number '"; // + s.substr(0, i + 1) + "'";

        i++;
    }
    if (i == 0)                              // что-либо похожее на число мы не нашли
        throw "can't get valid number in '"; // + s + "'";

    double dPart = toDouble(s.substr(0, i));
    if (negative)
        dPart = -dPart;

    sv restPart = s.substr(i);

    return Result(dPart, restPart);
}

Result MathParser::processFunction(sv func, const Result& r) {

    //    if (func.starts_with("sin"))
    //        return Result(sin(r.acc), r.rest);
    //    if (func.starts_with("cos"))
    //        return Result(cos(r.acc), r.rest);
    //    if (func.starts_with("tan"))
    //        return Result(tan(r.acc), r.rest);
    //    return r;

    enum class Func {
        sin,
        cos,
        tan
    };
    switch (Func(QString("sin,cos,tan").split(',').indexOf(toString(func)))) {
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
