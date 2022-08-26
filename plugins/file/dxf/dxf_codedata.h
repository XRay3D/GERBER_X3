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
#pragma once

#include <QDebug>
#include <QObject>
#include <QVariant>
#include <variant>

namespace Dxf {

template <typename T>
struct TypenameTest;

class CodeData;
using variant = std::variant<int16_t, int32_t, int64_t, double, QString>;
using Codes = std::vector<CodeData>;

class CodeData {
    Q_GADGET
    int lineNum;
    int code_ = 0;
    variant varVal;
    QString strVal;

public:
    CodeData(int code = 0, const QString& val = {}, int lineNum = 0);

    friend QDebug operator<<(QDebug debug, const CodeData& c);

    enum Type {
        Integer16,
        Integer32,
        Integer64,
        Double,
        String,
    };
    Q_ENUM(Type)

    int code() const;
    int line() const;
    QString string() const;

    template <class>
    inline static constexpr bool always_false_v = false;

    template <typename T>
    inline operator T() const {
        return std::visit([this](auto&& arg) -> std::decay_t<T> {
            using To = std::decay_t<T>;
            using Fr = std::decay_t<decltype(arg)>;
            bool ok = true;
            T val;
            /**/ if constexpr (std::is_same_v<Fr, To>)
                return arg;
            else if constexpr (std::is_same_v<To, QString> && std::is_integral_v<Fr>)
                return QString::number(arg);
            else if constexpr (std::is_same_v<To, QString> && std::is_floating_point_v<Fr>)
                return QString::number(arg);
            else if constexpr (std::is_same_v<Fr, QString> && std::is_integral_v<To>)
                val = T(arg.toLongLong(&ok));
            else if constexpr (std::is_same_v<Fr, QString> && std::is_floating_point_v<To>)
                val = T(arg.toDouble(&ok));
            else if constexpr (std::is_integral_v<To>)
                return T(arg);
            else if constexpr (std::is_floating_point_v<To>)
                return T(arg);
            else
                TypenameTest<T> {}; // static_assert(always_false_v<T>, "non-exhaustive visitor!");
            if (!ok)
                throw QString("CodeData::operator T(), %1 to %2 from %3")
                    .arg(typeid(Fr).name())
                    .arg(typeid(std::decay_t<T>).name())
                    .arg(strVal);
            return val;
        },
            varVal);
    }

    friend bool operator==(const CodeData& l, const QString& r) { return l.strVal == r; }

    friend bool operator!=(const CodeData& l, const QString& r) { return !(l == r); }
    friend bool operator==(const QString& l, const CodeData& r) { return l == r.strVal; }
    friend bool operator!=(const QString& l, const CodeData& r) { return !(l == r); }

    friend bool operator==(const CodeData& l, const char* r) { return l.strVal == r; }
    friend bool operator!=(const CodeData& l, const char* r) { return !(l == r); }
    friend bool operator==(const char* l, const CodeData& r) { return l == r.strVal; }
    friend bool operator!=(const char* l, const CodeData& r) { return !(l == r); }

    Type type() const;
    QVariant value() const;
};

} // namespace Dxf
