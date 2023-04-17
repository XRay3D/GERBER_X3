///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  March 25, 2023                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2023                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
///**
// * @link   https://habrahabr.ru/post/122397/
// * @author shurik
// */
// #pragma once

// #include <QMap>
// #include <QObject>
// #include <string_view>

// using VarMap = std::map<QString, double>;
// using sv = std::u16string_view;

// class Result {
// public:
//     double acc; // Аккамулятор
//     sv rest;    // остаток строки, которую мы еще не обработали
//     Result(double v = {}, sv rest = {})
//         : acc(v)
//         , rest(rest = {}) {
//     }
// };

// class MathParser {
// public:
//     MathParser(VarMap* variables);
//     MathParser() = default;
//     double getVariable(const QString& varName);
//     double parse(const QString& s = {});

// private:
//     QString toString(sv s) { return QString((const QChar*)s.data(), s.size()); }

//    double toDouble(sv s) { //    double val;        //    std::frochars_(s.data(), s.data() + s.size(), val);
//        return toString(s).toDouble();
//    }

//    VarMap* variables {nullptr};
//    Result plusMinus(sv s);
//    Result bracket(sv s);
//    Result functionVariable(sv s);
//    Result mulDiv(sv s);
//    Result num(sv s);
//    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
//    Result processFunction(sv func, const Result& r);
//};
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */
#pragma once

#include <QObject>

using VarMap = std::map<QString, double>;

class Result {
public:
    double acc{};       // Аккамулятор
    QStringView rest{}; // остаток строки, которую мы еще не обработали
    //    Result(double v = 0.0, const QStringView& r = {})
    //        : acc(v)
    //        , rest(r) {
    //    }
};

class MathParser {
public:
    MathParser(VarMap* variables);
    double getVariable(QStringView variableName);
    double parse(const QString& s = "");

private:
    VarMap* variables{nullptr};
    Result plusMinus(QStringView s);
    Result bracket(QStringView s);
    Result functionVariable(QStringView s);
    Result mulDiv(QStringView s);
    Result num(QStringView s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(QStringView func, Result r);
};
