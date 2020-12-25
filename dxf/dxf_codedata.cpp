#include "dxf_codedata.h"

namespace Dxf {

CodeData::CodeData(int code, const QString& value, int lineNum)
    : lineNum(lineNum)
    , m_code(code)
    , strVal(value)
{
    if (0 <= code && code <= 9) /*                String (with the introduction of extended symbol names in AutoCAD 2000, the 255-character
                                                    limit has been increased to 2049 single-byte characters not including the newline at the end
                                                    of the line)*/
        varVal = strVal;
    else if (10 <= code && code <= 39) //       Double precision 3D point value
        varVal = value.toDouble();
    else if (40 <= code && code <= 59) //       Double-precision floating-point value
        varVal = value.toDouble();
    else if (60 <= code && code <= 79) //       16-bit integer value
        varVal = value.toLongLong();
    else if (90 <= code && code <= 99) //       32-bit integer value
        varVal = value.toLongLong();
    else if (code == 100) //                    String (255-character maximum; less for Unicode strings)
        varVal = strVal;
    else if (code == 102) //                    String (255-character maximum; less for Unicode strings)
        varVal = strVal;
    else if (code == 105) //                    String representing hexadecimal (hex) handle value
        varVal = strVal;
    else if (110 <= code && code <= 119) //     Double precision floating-point value
        varVal = value.toDouble();
    else if (120 <= code && code <= 129) //     Double precision floating-point value
        varVal = value.toDouble();
    else if (130 <= code && code <= 139) //     Double precision floating-point value
        varVal = value.toDouble();
    else if (140 <= code && code <= 149) //     Double precision scalar floating-point value
        varVal = value.toDouble();
    else if (160 <= code && code <= 169) //     64-bit integer value
        varVal = value.toLongLong();
    else if (170 <= code && code <= 179) //     16-bit integer value
        varVal = value.toLongLong();
    else if (210 <= code && code <= 239) //     Double-precision floating-point value
        varVal = value.toDouble();
    else if (270 <= code && code <= 279) //     16-bit integer value
        varVal = value.toLongLong();
    else if (280 <= code && code <= 289) //     16-bit integer value
        varVal = value.toLongLong();
    else if (290 <= code && code <= 299) //     Boolean flag value
        varVal = value.toLongLong();
    else if (300 <= code && code <= 309) //     Arbitrary text string
        varVal = strVal;
    else if (310 <= code && code <= 319) //     String representing hex value of binary chunk
        varVal = strVal;
    else if (320 <= code && code <= 329) //     String representing hex handle value
        varVal = strVal;
    else if (330 <= code && code <= 369) //     String representing hex object IDs
        varVal = strVal;
    else if (370 <= code && code <= 379) //     16-bit integer value
        varVal = value.toLongLong();
    else if (380 <= code && code <= 389) //     16-bit integer value
        varVal = value.toLongLong();
    else if (390 <= code && code <= 399) //     String representing hex handle value
        varVal = strVal;
    else if (400 <= code && code <= 409) //     16-bit integer value
        varVal = value.toLongLong();
    else if (410 <= code && code <= 419) //     String
        varVal = strVal;
    else if (420 <= code && code <= 429) //     32-bit integer value
        varVal = value.toLongLong();
    else if (430 <= code && code <= 439) //     String
        varVal = strVal;
    else if (440 <= code && code <= 449) //     32-bit integer value
        varVal = value.toLongLong();
    else if (450 <= code && code <= 459) //     Long
        varVal = value.toLongLong();
    else if (460 <= code && code <= 469) //     Double-precision floating-point value
        varVal = value.toDouble();
    else if (470 <= code && code <= 479) //     String
        varVal = strVal;
    else if (480 <= code && code <= 481) //     String representing hex handle value
        varVal = strVal;
    else if (code == 999) //                    Comment (string)
        varVal = strVal;
    else if (1000 <= code && code <= 1009) //   String (same limits as indicated with 0-9 code range)
        varVal = strVal;
    else if (1010 <= code && code <= 1059) //   Double-precision floating-point value
        varVal = value.toDouble();
    else if (1060 <= code && code <= 1070) //   16-bit integer value
        varVal = value.toLongLong();
    else if (code == 1071) //                   32-bit integer value
        varVal = value.toLongLong();
    else
        throw QString("Unknown data: code %1, data %2, line %3!").arg(code).arg(value).arg(lineNum);
}

int CodeData::code() const { return m_code; }

int CodeData::line() const { return lineNum; }

QString CodeData::string() const { return strVal; }

CodeData::Type CodeData::type() const { return static_cast<Type>(varVal.index()); }

QVariant CodeData::value() const
{
    return std::visit([](auto&& arg) -> QVariant { return arg; }, varVal);
}

CodeData::operator double() const
{
    try {
        return std::get<double>(varVal);
    } catch (const std::bad_variant_access& ex) {
        qDebug() << ex.what();
        throw QString("Bad variant access: %1, line num %2").arg(ex.what()).arg(lineNum);
    }
}

CodeData::operator int64_t() const
{
    try {
        return std::get<int64_t>(varVal);
    } catch (const std::bad_variant_access& ex) {
        qDebug() << ex.what();
        throw QString("Bad variant access: %1, line num %2").arg(ex.what()).arg(lineNum);
    }
}

CodeData::operator int() const
{
    try {
        return std::get<int64_t>(varVal);
    } catch (const std::bad_variant_access& ex) {
        qDebug() << ex.what();
        throw QString("Bad variant access: %1, line num %2").arg(ex.what()).arg(lineNum);
    }
}

CodeData::operator QString() const
{
    try {
        return std::get<QString>(varVal);
    } catch (const std::bad_variant_access& ex) {
        qDebug() << ex.what();
        throw QString("Bad variant access: %1, line num %2").arg(ex.what()).arg(lineNum);
    }
}

}
