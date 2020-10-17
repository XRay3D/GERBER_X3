// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */
#include "mathparser.h"
#include <QDebug>
#include <QtMath>

MathParser::MathParser(QMap<QString, double>& variables)
    : variables(&variables)
{
}

double MathParser::getVariable(QString variableName)
{
    if (!variables->contains(variableName)) {
        qWarning() << "Error: Try get unexists variable '" + variableName + "'";
        return 0.0;
    }
    return variables->value(variableName, 0.0);
}

double MathParser::parse(const QString& s)
{
    Result result;
    try {
        result = plusMinus(s);
        if (!result.rest.isEmpty()) {
            qWarning() << "Error: can't full parse"
                       << "\"" << s << "\"";
            qWarning() << "rest: " + result.rest;
        }
    } catch (const QString& str) {
        qWarning() << str;
    }
    return result.acc;
}

Result MathParser::plusMinus(QString s) //throws Exception
{
    Result current = mulDiv(s);
    double acc = current.acc;

    while (current.rest.length() > 0) {
        if (!(current.rest.at(0) == '+' || current.rest.at(0) == '-'))
            break;

        QChar sign = current.rest.at(0);
        QString next = current.rest.mid(1);

        current = mulDiv(next);
        if (sign == '+')
            acc += current.acc;
        else
            acc -= current.acc;
    }
    return Result(acc, current.rest);
}

Result MathParser::bracket(QString s) //throws Exception
{
    QChar zeroChar = s.at(0);
    if (zeroChar == '(') {
        Result r = plusMinus(s.mid(1));
        if (!r.rest.isEmpty() && r.rest.at(0) == ')')
            r.rest = r.rest.mid(1);
        else
            qWarning() << "Error: not close bracket";
        return r;
    }
    return functionVariable(s);
}

Result MathParser::functionVariable(QString s) //throws Exception
{
    QString f;
    int i = 0;
    int sign = 1;
    if (s.startsWith('-')) {
        sign = -1;
        s.remove(0, 1);
    }
    // ищем название функции или переменной
    // имя обязательно должна начинаться с буквы
    while (i < s.length() && ((s.at(i).isLetter() || s.at(i) == '$') || (s.at(i).isDigit() && i > 0))) {
        //while (i < s.length() && (s.at(i).isLetter() || (s.at(i).isDigit() && i > 0))) {
        f += s.at(i);
        i++;
    }
    if (!f.isEmpty()) { // если что-нибудь нашли
        if (s.length() > i && s.at(i) == '(') { // и следующий символ скобка значит - это функция
            Result r = bracket(s.mid(f.length()));
            return processFunction(f, r);
        } else // иначе - это переменная
            return Result(getVariable(f) * sign, s.mid(f.length()));
    }
    return num(s);
}

Result MathParser::mulDiv(QString s) //throws Exception
{
    Result current = bracket(s);

    double acc = current.acc;
    while (true) {
        if (current.rest.length() == 0)
            return current;

        QChar sign = current.rest.at(0);
        if ((sign != '*' && sign != '/'))
            return current;

        QString next = current.rest.mid(1);
        Result right = bracket(next);

        if (sign == '*')
            acc *= right.acc;
        else
            acc /= right.acc;

        current = Result(acc, right.rest);
    }
}

Result MathParser::num(QString s) //throws Exception
{
    int i = 0;
    int dot_cnt = 0;
    bool negative = false;
    // число также может начинаться с минуса
    if (s.at(0) == '-') {
        negative = true;
        s = s.mid(1);
    }
    // разрешаем только цифры и точку
    while (i < s.length() && (s.at(i).isDigit() || s.at(i) == '.')) {
        // но также проверям, что в числе может быть только одна точка!
        if (s.at(i) == '.' && ++dot_cnt > 1) {
            throw "not valid number '" + s.mid(0, i + 1) + "'";
        }
        i++;
    }
    if (i == 0) // что-либо похожее на число мы не нашли
        throw "can't get valid number in '" + s + "'";

    double dPart = s.mid(0, i).toDouble();
    if (negative)
        dPart = -dPart;

    QString restPart = s.mid(i);

    return Result(dPart, restPart);
}

Result MathParser::processFunction(QString func, Result r)
{
    enum {
        sin,
        cos,
        tan
    };
    switch (QString("sin,cos,tan").split(',').indexOf(func)) {
    case sin:
        return Result(qSin(r.acc), r.rest);
    case cos:
        return Result(qCos(r.acc), r.rest);
    case tan:
        return Result(qTan(r.acc), r.rest);
    default:
        qWarning() << "function '" + func + "' is not defined";
        break;
    }
    return r;
}
