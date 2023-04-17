// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
    : lineNum(lineNum)
    , code_(code)
    , strVal(value) {
    bool ok = true;
    Type type;
    if(0 <= code && code <= 9) //            Строка (с появлением расширенных имен символов в AutoCAD 2000 предел в 255 символов был увеличен
        //                                    до 2049 однобайтовых символов без учета символа абзаца в конце строки).
        //                                    Подробные сведения см. в разделе «Хранение строковых значений».
        type = String;
    else if(10 <= code && code <= 39)     //     Значение 3D-точки двойной точности
        type = Double;
    else if(40 <= code && code <= 59)     //     Значение с плавающей запятой двойной точности
        type = Double;
    else if(60 <= code && code <= 79)     //     16-разрядное целое значение
        type = Integer16;
    else if(90 <= code && code <= 99)     //     32-разрядное целое значение
        type = Integer32;
    else if(100 == code)                  //                  Строка (максимум 255 символов, меньше для строк в формате Юникод). Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(102 == code)                  //                  Строка (максимум 255 символов, меньше для строк в формате Юникод). Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(105 == code)                  //                  Строка, представляющая шестнадцатеричное значение дескриптора
        type = String;
    else if(110 <= code && code <= 119)   //   Значение с плавающей запятой двойной точности
        type = Double;
    else if(120 <= code && code <= 129)   //   Значение с плавающей запятой двойной точности
        type = Double;
    else if(130 <= code && code <= 139)   //   Значение с плавающей запятой двойной точности
        type = Double;
    else if(140 <= code && code <= 149)   //   Скалярное значение с плавающей запятой двойной точности
        type = Double;
    else if(160 <= code && code <= 169)   //   64-разрядное целое значение
        type = Integer64;
    else if(170 <= code && code <= 179)   //   16-разрядное целое значение
        type = Integer16;
    else if(210 <= code && code <= 239)   //   Значение с плавающей запятой двойной точности
        type = Double;
    else if(270 <= code && code <= 279)   //   16-разрядное целое значение
        type = Integer16;
    else if(280 <= code && code <= 289)   //   16-разрядное целое значение
        type = Integer16;
    else if(290 <= code && code <= 299)   //   Логическое значение флага
        type = String;
    else if(300 <= code && code <= 309)   //   Произвольная текстовая строка. Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(310 <= code && code <= 319)   //   Строка, представляющая шестнадцатеричное значение двоичного уровня
        type = String;
    else if(320 <= code && code <= 329)   //   Строка, представляющая шестнадцатеричное значение дескриптора
        type = String;
    else if(330 <= code && code <= 369)   //   Строка, представляющая шестнадцатеричное значение идентификаторов объектов
        type = String;
    else if(370 <= code && code <= 379)   //   16-разрядное целое значение
        type = Integer16;
    else if(380 <= code && code <= 389)   //   16-разрядное целое значение
        type = Integer16;
    else if(390 <= code && code <= 399)   //   Строка, представляющая шестнадцатеричное значение дескриптора
        type = String;
    else if(400 <= code && code <= 409)   //   16-разрядное целое значение
        type = Integer16;
    else if(410 <= code && code <= 419)   //   Строка. Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(420 <= code && code <= 429)   //   32-разрядное целое значение
        type = Integer32;
    else if(430 <= code && code <= 439)   //   Строка. Подробные сведения см. в разделе «Хранение строковых значений»
        type = Integer64;
    else if(440 <= code && code <= 449)   //   32-разрядное целое значение
        type = Integer32;
    else if(450 <= code && code <= 459)   //   Long
        type = Integer64;
    else if(460 <= code && code <= 469)   //   Значение с плавающей запятой двойной точности
        type = Double;
    else if(470 <= code && code <= 479)   //   Строка. Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(480 <= code && code <= 481)   //   Строка, представляющая шестнадцатеричное значение дескриптора
        type = String;
    else if(999 == code)                  //                  Комментарий (строка). Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(1000 <= code && code <= 1009) // Комментарий (ограничения аналогичны указанным для кодов в диапазоне 0–9). Подробные сведения см. в разделе «Хранение строковых значений»
        type = String;
    else if(1010 <= code && code <= 1059) // Значение с плавающей запятой двойной точности
        type = Double;
    else if(1060 <= code && code <= 1070) // 16-разрядное целое значение
        type = Integer16;
    else if(1071 == code)                 //                 32-разрядное целое значение
        type = Integer32;
    else
        throw QString("Unknown data: code %1, data %2, line %3!").arg(code).arg(value).arg(lineNum);

    switch(type) {
    case Integer16:
        varVal = int16_t(value.toLongLong(&ok));
        break;
    case Integer32:
        varVal = int32_t(value.toLongLong(&ok));
        break;
    case Integer64:
        varVal = int64_t(value.toLongLong(&ok));
        break;
    case Double:
        varVal = value.toDouble(&ok);
        break;
    case String:
        varVal = value;
    }
    if(!ok)
        switch(type) {
        case Integer16:
            varVal = int16_t(value.toDouble(&ok));
            break;
        case Integer32:
            varVal = int32_t(value.toDouble(&ok));
            break;
        case Integer64:
            varVal = int64_t(value.toDouble(&ok));
            break;
        case Double:
            varVal = value.toDouble(&ok);
            break;
        case String:
            varVal = value;
        }

    if(!ok)
        throw QString("Unknown data: code %1, data %2, line %3!").arg(code).arg(value).arg(lineNum);
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
