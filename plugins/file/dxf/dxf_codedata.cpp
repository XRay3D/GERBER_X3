/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_codedata.h"

namespace Dxf {

QDebug operator<<(QDebug debug, const CodeData& c) {
    QDebugStateSaver saver(debug);
    //      debug.nospace() << QString("DC(%1, ").arg(c.code_, 5).toUtf8().data();
    //      debug.nospace() << '\n';
    debug.nospace() << QString("DC(%1, ").arg(c.code_).toUtf8().data();
    std::visit([&debug](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr /*  */ (std::is_same_v<T, double>)
            debug << "F64 ";
        else if constexpr(std::is_same_v<T, int16_t>)
            debug << "I16 ";
        else if constexpr(std::is_same_v<T, int32_t>)
            debug << "I32 ";
        else if constexpr(std::is_same_v<T, int64_t>)
            debug << "I64 ";
        else if constexpr(std::is_same_v<T, QString>)
            debug << "Str ";
        debug << arg;
    },
        c.varVal);
    debug.nospace() << ')';
    return debug;
}

CodeData::CodeData(int code, const QString& value, int lineNum)
    : lineNum{lineNum}
    , code_(code)
    , strVal(value) {
    bool ok = true;
    Type type;
#if 1
    if(0 <= code && code <= 9) type = String;               // Строка (с появлением расширенных имен символов в AutoCAD 2000 предел в 255 символов был увеличен
                                                            // до 2049 однобайтовых символов без учета символа абзаца в конце строки).
                                                            // Подробные сведения см. в разделе «Хранение строковых значений».
    else if(10 <= code && code <= 39) type = Double;        // Значение 3D-точки двойной точности
    else if(40 <= code && code <= 59) type = Double;        // Значение с плавающей запятой двойной точности
    else if(60 <= code && code <= 79) type = Integer16;     // 16-разрядное целое значение
    else if(90 <= code && code <= 99) type = Integer32;     // 32-разрядное целое значение
    else if(100 == code) type = String;                     // Строка (максимум 255 символов, меньше для строк в формате Юникод). Подробные сведения см. в разделе «Хранение строковых значений»
    else if(102 == code) type = String;                     // Строка (максимум 255 символов, меньше для строк в формате Юникод). Подробные сведения см. в разделе «Хранение строковых значений»
    else if(105 == code) type = String;                     // Строка, представляющая шестнадцатеричное значение дескриптора
    else if(110 <= code && code <= 119) type = Double;      // Значение с плавающей запятой двойной точности
    else if(120 <= code && code <= 129) type = Double;      // Значение с плавающей запятой двойной точности
    else if(130 <= code && code <= 139) type = Double;      // Значение с плавающей запятой двойной точности
    else if(140 <= code && code <= 149) type = Double;      // Скалярное значение с плавающей запятой двойной точности
    else if(160 <= code && code <= 169) type = Integer64;   // 64-разрядное целое значение
    else if(170 <= code && code <= 179) type = Integer16;   // 16-разрядное целое значение
    else if(210 <= code && code <= 239) type = Double;      // Значение с плавающей запятой двойной точности
    else if(270 <= code && code <= 279) type = Integer16;   // 16-разрядное целое значение
    else if(280 <= code && code <= 289) type = Integer16;   // 16-разрядное целое значение
    else if(290 <= code && code <= 299) type = String;      // Логическое значение флага
    else if(300 <= code && code <= 309) type = String;      // Произвольная текстовая строка. Подробные сведения см. в разделе «Хранение строковых значений»
    else if(310 <= code && code <= 319) type = String;      // Строка, представляющая шестнадцатеричное значение двоичного уровня
    else if(320 <= code && code <= 329) type = String;      // Строка, представляющая шестнадцатеричное значение дескриптора
    else if(330 <= code && code <= 369) type = String;      // Строка, представляющая шестнадцатеричное значение идентификаторов объектов
    else if(370 <= code && code <= 379) type = Integer16;   // 16-разрядное целое значение
    else if(380 <= code && code <= 389) type = Integer16;   // 16-разрядное целое значение
    else if(390 <= code && code <= 399) type = String;      // Строка, представляющая шестнадцатеричное значение дескриптора
    else if(400 <= code && code <= 409) type = Integer16;   // 16-разрядное целое значение
    else if(410 <= code && code <= 419) type = String;      // Строка. Подробные сведения см. в разделе «Хранение строковых значений»
    else if(420 <= code && code <= 429) type = Integer32;   // 32-разрядное целое значение
    else if(430 <= code && code <= 439) type = Integer64;   // Строка. Подробные сведения см. в разделе «Хранение строковых значений»
    else if(440 <= code && code <= 449) type = Integer32;   // 32-разрядное целое значение
    else if(450 <= code && code <= 459) type = Integer64;   // Long
    else if(460 <= code && code <= 469) type = Double;      // Значение с плавающей запятой двойной точности
    else if(470 <= code && code <= 479) type = String;      // Строка. Подробные сведения см. в разделе «Хранение строковых значений»
    else if(480 <= code && code <= 481) type = String;      // Строка, представляющая шестнадцатеричное значение дескриптора
    else if(999 == code) type = String;                     // Комментарий (строка). Подробные сведения см. в разделе «Хранение строковых значений»
    else if(1000 <= code && code <= 1009) type = String;    // Комментарий (ограничения аналогичны указанным для кодов в диапазоне 0–9). Подробные сведения см. в разделе «Хранение строковых значений»
    else if(1010 <= code && code <= 1059) type = Double;    // Значение с плавающей запятой двойной точности
    else if(1060 <= code && code <= 1070) type = Integer16; // 16-разрядное целое значение
    else if(1071 == code) type = Integer32;                 // 32-разрядное целое значение
    else throw QString("Unknown type: code %1, raw %2, line %3!").arg(code).arg(value).arg(lineNum);
#else

    if(0 <= code && code <= 9) type = String;                         // String
    else if(10 <= code && code <= 39) type = Double;                  // Double precision 3D point value
    else if(40 <= code && code <= 59) type = Double;                  // Double-precision floating-point value
    else if(60 <= code && code <= 79) type = Integer16;               // 16-bit integer value
    else if(90 <= code && code <= 99) type = Integer32;               // 32-bit integer value
    else if(100 == code) type = String;                               // String
    else if(102 == code) type = String;                               // String
    else if(105 == code) type = String;                               // String representing hexadecimal (hex) handle value
    else if(110 <= code && code <= 119) type = Double;                // Double precision floating-point value
    else if(120 <= code && code <= 129) type = Double;                // Double precision floating-point value
    else if(130 <= code && code <= 139) type = Double;                // Double precision floating-point value
    else if(140 <= code && code <= 149) type = Double;                // Double precision scalar floating-point value
    else if(160 <= code && code <= 169) type = Integer64;             // 64-bit integer value
    else if(170 <= code && code <= 179) type = Integer16;             // 16-bit integer value
    else if(210 <= code && code <= 239) type = Double;                // Double-precision floating-point value
    else if(270 <= code && code <= 279) type = Integer16;             // 16-bit integer value
    else if(280 <= code && code <= 289) type = Integer16;             // 16-bit integer value
    else if(290 <= code && code <= 299) type = Integer16 /*Boolean*/; // Boolean flag value
    else if(300 <= code && code <= 309) type = String /*Arbitrary*/;  // Arbitrary text string
    else if(310 <= code && code <= 319) type = String;                // String representing hex value of binary chunk
    else if(320 <= code && code <= 329) type = String /*Arbitrary*/;  // Arbitrary pointer, hex object ID, not translated during INSERT and XREF operations
    else if(330 <= code && code <= 339) type = String /*Soft*/;       // Soft-pointer, hex object ID, translated during INSERT and XREF operations
    else if(340 <= code && code <= 349) type = String /*Hard*/;       // Hard-pointer, hex object ID, translated during INSERT and XREF operations
    else if(350 <= code && code <= 359) type = String /*Soft*/;       // Soft-owner, hex object ID, translated during INSERT and XREF operations
    else if(360 <= code && code <= 369) type = String /*Hard*/;       // Hard-owner, hex object ID, translated during INSERT and XREF operations
    else if(370 <= code && code <= 379) type = Integer16;             // 16-bit integer value
    else if(380 <= code && code <= 389) type = Integer16;             // 16-bit integer value
    else if(390 <= code && code <= 399) type = String;                // String representing hex handle value
    else if(400 <= code && code <= 409) type = Integer16;             // 16-bit integer value
    else if(410 <= code && code <= 419) type = String;                // String
    else if(420 <= code && code <= 429) type = Integer32;             // 32-bit integer value
    else if(430 <= code && code <= 439) type = String;                // String
    else if(440 <= code && code <= 449) type = Integer32;             // 32-bit integer value
    else if(450 <= code && code <= 459) type = Integer64 /*Long*/;    // Long
    else if(460 <= code && code <= 469) type = Double;                // Double-precision floating-point value
    else if(470 <= code && code <= 479) type = String;                // String
    else if(480 <= code && code <= 481) type = String /*Hard*/;       // Hard-pointer, hex object ID, translated during INSERT and XREF operations
    else if(999 == code) type = String /*Comment*/;                   // Comment (string)
    else if(1000 <= code && code <= 1009) type = String;              // String
    else if(1010 <= code && code <= 1059) type = Double;              // Double-precision floating-point value
    else if(1060 <= code && code <= 1070) type = Integer16;           // 16-bit integer value
    else if(1071 == code) type = Integer32;                           // 32-bit integer value

#endif

    switch(type) {
    case Integer16: varVal = int16_t(value.toLongLong(&ok)); break;
    case Integer32: varVal = int32_t(value.toLongLong(&ok)); break;
    case Integer64: varVal = int64_t(value.toLongLong(&ok)); break;
    case Double: varVal = value.toDouble(&ok); break;
    case String: varVal = value;
    }

    if(!ok) {
        switch(type) {
        case Integer16: varVal = int16_t(value.toDouble(&ok)); break;
        case Integer32: varVal = int32_t(value.toDouble(&ok)); break;
        case Integer64: varVal = int64_t(value.toDouble(&ok)); break;
        case Double: varVal = value.toDouble(&ok); break;
        case String: varVal = value;
        }
        qWarning().nospace() << "Type missmatch: code " << code << ", type " << type << ", raw " << value << ", line " << lineNum << "!";
    }

    if(!ok)
        throw QString("Error value: code %1, raw %2, line %3!").arg(code).arg(value).arg(lineNum);
}

int CodeData::code() const { return code_; }

int CodeData::line() const { return lineNum; }

QString CodeData::string() const { return strVal; }

CodeData::Type CodeData::type() const { return static_cast<Type>(varVal.index()); }

QVariant CodeData::value() const {
    return std::visit([](auto&& arg) { return QVariant::fromValue(arg); }, varVal);
}

} // namespace Dxf

#include "moc_dxf_codedata.cpp"
