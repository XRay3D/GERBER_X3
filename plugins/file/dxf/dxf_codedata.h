/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "utils.h"
#include <QDebug>
#include <QObject>
#include <QVariant>
#include <concepts>
#include <variant>

namespace Dxf {

template <typename T>
struct TypenameTest;

class CodeData;
using variant = Variant<int16_t, int32_t, int64_t, double, QString>;
using Codes = std::vector<CodeData>;

class CodeData {
    Q_GADGET
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
        using To = std::decay_t<T>;
        return varVal.visit(
            [](std::integral auto val) -> To {
                if constexpr(std::is_same_v<To, QString>)
                    return QString::number(val);
                else return val;
            },
            [](std::floating_point auto val) -> To {
                if constexpr(std::is_same_v<To, QString>)
                    return QString::number(val);
                else return val;
            },
            [](const QString& val) -> To {
                if constexpr(!std::is_same_v<To, QString>)
                    return QVariant{val}.value<To>();
                else return val;
            });
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

private:
    int lineNum{};
    int code_{};
    variant varVal;
    QString strVal;
    Type type_{};
};

} // namespace Dxf
