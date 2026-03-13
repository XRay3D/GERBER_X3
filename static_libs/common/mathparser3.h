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
#pragma once

#include <QObject>

class MathParser3 {
public:
    class Result {
    public:
        double acc{};       // Аккамулятор
        QStringView rest{}; // остаток строки, которую мы еще не обработали
        // Result(double v = 0.0, const QStringView& r = {})
        // : acc{v}
        // , rest(r) {
        // }
    };
    using VarMap = std::map<QString, double>;
    MathParser3(VarMap* variables);
    double getVariable(QStringView variableName);
    double parse(const QString& s = {});
    double parse(QStringView s = {});

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
