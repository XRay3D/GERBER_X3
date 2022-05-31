/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
/**
 * https://habrahabr.ru/post/122397/
 * @author shurik
 */
#pragma once
#ifndef MATCHPARSER_H
#define MATCHPARSER_H

#include <QMap>
#include <QObject>

class Result {
public:
    double acc;   // Аккамулятор
    QString rest; // остаток строки, которую мы еще не обработали
    Result(double v = 0.0, const QString& r = "")
        : acc(v)
        , rest(r) {
    }
};

class MathParser {
public:
    MathParser(QMap<QString, double>& variables);
    double getVariable(QString variableName);
    double parse(const QString& s = "");

private:
    QMap<QString, double>* variables;
    Result plusMinus(QString s);
    Result bracket(QString s);
    Result functionVariable(QString s);
    Result mulDiv(QString s);
    Result num(QString s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(QString func, Result r);
};

#endif // MATCHPARSER_H
