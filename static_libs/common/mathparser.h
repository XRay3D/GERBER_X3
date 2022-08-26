/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */
#pragma once

#include <QMap>
#include <QObject>
#include <string_view>

using VarMap = std::map<QString, double>;
using sv = std::u16string_view;

class Result {
public:
    double acc; // Аккамулятор
    sv rest;    // остаток строки, которую мы еще не обработали
    Result(double v = {}, sv rest = {})
        : acc(v)
        , rest(rest = {}) {
    }
};

class MathParser {
public:
    MathParser(VarMap* variables);
    MathParser() { }
    double getVariable(const QString& varName);
    double parse(const QString& s = {});

private:
    QString toString(sv s) { return QString((const QChar*)s.data(), s.size()); }

    double toDouble(sv s) { //    double val;        //    std::frochars_(s.data(), s.data() + s.size(), val);
        return toString(s).toDouble();
    }

    VarMap* variables = {};
    Result plusMinus(sv s);
    Result bracket(sv s);
    Result functionVariable(sv s);
    Result mulDiv(sv s);
    Result num(sv s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(sv func, Result r);
};
