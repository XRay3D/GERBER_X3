#pragma once

#include <QDebug>
#include <QObject>

namespace Dxf {

using variant = std::variant<double, qint64, QString>;

class CodeData {
    Q_GADGET
    int lineNum;
    int m_code = 0;
    variant varVal;
    QString strVal;

public:
    CodeData(int code = 0, const QString& val = {}, int lineNum = 0);

    friend QDebug operator<<(QDebug debug, const CodeData& c)
    {
        QDebugStateSaver saver(debug);
        //        debug.nospace() << QString("DC(%1, ").arg(c.m_code, 5).toLocal8Bit().data();
        debug.nospace() << '\n';
        debug.nospace() << QString("DC(%1, ").arg(c.m_code).toLocal8Bit().data();
        std::visit([&debug](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            /*  */ if constexpr (std::is_same_v<T, double>) {
                debug << "D ";
            } else if constexpr (std::is_same_v<T, qint64>) {
                debug << "I ";
            } else if constexpr (std::is_same_v<T, QString>) {
                debug << "S ";
            }
            debug << arg;
        },
            c.varVal);
        debug.nospace() << ')';
        return debug;
    }

    enum Type {
        Double,
        Integer,
        String,
    };
    Q_ENUM(Type)

    int code() const;

    operator double() const;
    operator qint64() const;
    operator int() const;
    operator QString() const;

    friend bool operator==(const CodeData& l, const QString& r) { return l.strVal == r; }
    friend bool operator!=(const CodeData& l, const QString& r) { return !(l == r); }
    friend bool operator==(const QString& l, const CodeData& r) { return l == r.strVal; }
    friend bool operator!=(const QString& l, const CodeData& r) { return !(l == r); }

    friend bool operator==(const CodeData& l, const char* r) { return l.strVal == r; }
    friend bool operator!=(const CodeData& l, const char* r) { return !(l == r); }
    friend bool operator==(const char* l, const CodeData& r) { return l == r.strVal; }
    friend bool operator!=(const char* l, const CodeData& r) { return !(l == r); }

    //    template <typename T = int, typename std::enable_if_t<std::is_integral_v<T>>>
    //    operator T() const
    //    {
    //        try {
    //            return static_cast<T>(std::get<qint64>(m_val));
    //        } catch (const std::bad_variant_access& ex) {
    //            qDebug() << ex.what();
    //            return -std::numeric_limits<T>::max();
    //        }
    //    }

    Type type() const;
    QVariant value() const;
};

} // namespace Dxf
