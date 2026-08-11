#include "utils.h"

#include "geo/util.h"

#include <unicode/ucnv.h>   // ICU Conversion (для преобразования)
#include <unicode/ucsdet.h> // ICU Charset Detection
#include <unicode/unistr.h>

QString toQString(std::string_view cp1251Str) {
    UErrorCode error = U_ZERO_ERROR;
    QString utf16Buff{static_cast<qsizetype>(cp1251Str.size() * 2), QChar{}};
    auto ucnv = ucnv_open("windows-1251", &error);
    Finaly _{[ucnv] { ucnv_close(ucnv); }};
    int32_t utf16Length = ucnv_toUChars(ucnv,
        reinterpret_cast<UChar*>(utf16Buff.data()), utf16Buff.size(),
        cp1251Str.data(), cp1251Str.size(),
        &error);
    if(U_FAILURE(error)) {
        qWarning() << u_errorName(error);
        return {};
    }
    return utf16Buff.resize(utf16Length), utf16Buff;
}

std::string toCp1251(const QString& utf16Str) {
    UErrorCode error = U_ZERO_ERROR;
    const UChar* utf16Data = reinterpret_cast<const UChar*>(utf16Str.utf16());
    std::string cp1251Buf(utf16Str.size() * 2, '\0');
    auto ucnv = ucnv_open("windows-1251", &error);
    Finaly _{[ucnv] { ucnv_close(ucnv); }};
    auto cp1251Length = ucnv_fromUChars(ucnv, cp1251Buf.data(), cp1251Buf.size(), utf16Data, utf16Str.size(), &error);

    if(U_FAILURE(error)) {
        qWarning() << u_errorName(error);
        return {};
    }
    return cp1251Buf.resize(cp1251Length), cp1251Buf;
}

// ---------------------------------------------------------------------
// Обнаруживаем кодировку, используя ICU
// ---------------------------------------------------------------------
void detectEncoding(std::string_view data) {
    UErrorCode status = U_ZERO_ERROR;

    // 1. Создаём объект UCharsetDetector
    UCharsetDetector* detector = ucsdet_open(&status);
    if(U_FAILURE(status)) {
        qCritical() << "Ошибка создания UCharsetDetector: "
                    << u_errorName(status) << '\n';
        return;
    }

    // 2. Передаём буфер
    ucsdet_setText(detector,
        reinterpret_cast<const char*>(data.data()),
        static_cast<int32_t>(data.size()),
        &status);
    if(U_FAILURE(status)) {
        qCritical() << "Ошибка установки текста: "
                    << u_errorName(status) << '\n';
        ucsdet_close(detector);
        return;
    }

    // 3. Получаем лучшее совпадение
    const UCharsetMatch* match = ucsdet_detect(detector, &status);
    if(U_FAILURE(status) || match == nullptr) {
        qCritical() << "Кодировка не найдена: "
                    << u_errorName(status) << '\n';
        ucsdet_close(detector);
        return;
    }

    // 4. Читаем имя и уверенность
    const char* charset_name = ucsdet_getName(match, &status);
    if(U_FAILURE(status)) charset_name = "неизвестно";

    int confidence = ucsdet_getConfidence(match, &status);
    if(U_FAILURE(status)) confidence = 0;

    // 5. (Опционально) Если нужно, переводим в UTF‑8
    //    Например, чтобы вывести в консоль.
    std::string name{charset_name};

    qInfo() << "Обнаружена кодировка: " << name
            << " (уверенность: " << confidence << "%)\n";

    ucsdet_close(detector);
}

// QByteArray toUtf8(std::string_view cp1251Str) {
//     qWarning("TODO");
//     return {};
// }

constexpr auto bom = "\xEF\xBB\xBF";

bool hasBom(std::string_view data) {
    return data.starts_with(bom);
}

bool isValidUtf8(std::string_view data) noexcept {
    std::size_t i = 0;
    const std::size_t n = std::size(data);

    while(i < n) {
        uint8_t c = static_cast<uint8_t>(data[i]);

        /* 1‑байтовый ASCII */
        if(c < 0x80) {
            ++i;
            continue;
        }

        /* 2‑байтовая последовательность: 110xxxxx 10xxxxxx */
        if((c & 0xE0) == 0xC0) {
            if(i + 1 >= n) return false;
            if((static_cast<uint8_t>(data[i + 1]) & 0xC0) != 0x80)
                return false;
            // Не‑overlong: минимум 0xC2
            if(c == 0xC0 || c == 0xC1) return false;
            i += 2;
            continue;
        }

        /* 3‑байтовая последовательность: 1110xxxx 10xxxxxx 10xxxxxx */
        if((c & 0xF0) == 0xE0) {
            if(i + 2 >= n) return false;
            uint8_t c1 = static_cast<uint8_t>(data[i + 1]),
                    c2 = static_cast<uint8_t>(data[i + 2]);
            if((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return false;
            // Защита от суррогатных пар и overlong
            if(c == 0xE0 && (c1 & 0xE0) == 0x80) return false; // < 0x0800
            if(c == 0xED && (c1 & 0xE0) == 0xA0) return false; // суррогаты U+D800..U+DFFF
            i += 3;
            continue;
        }

        /* 4‑байтовая последовательность: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        if((c & 0xF8) == 0xF0) {
            if(i + 3 >= n) return false;
            uint8_t c1 = static_cast<uint8_t>(data[i + 1]),
                    c2 = static_cast<uint8_t>(data[i + 2]),
                    c3 = static_cast<uint8_t>(data[i + 3]);
            if((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80)
                return false;
            // Кодовая точка должна быть ≤ 0x10FFFF и минимум 0x10000
            if(c == 0xF0 && (c1 & 0xF0) == 0x80) return false; // < 0x10000
            if(c > 0xF4) return false;                         // > 0x10FFFF
            i += 4;
            continue;
        }

        /* Если дошли сюда – это непредусмотренный байт */
        return false;
    }
    return true;
}

QIcon drawIcon(const QPainterPath& pPath, QColor color, bool stroke) {
    auto rect = pPath.boundingRect();
    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());
    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    QPixmap pixmap{IconSize, IconSize};
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    if(stroke) {
        painter.strokePath(pPath, {color, 0.0});
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPath(pPath);
    }
    return pixmap;
}

QIcon drawIcon(const Geo::Polylines& polylines, QColor color, bool stroke) {
    return drawIcon(Geo::toPath(polylines), color, stroke);
}

QIcon drawIcon(const Geo::Polygons& polygons, QColor color) {
    // Путь региона уже несёт правило заполнения, при котором дырки вычитаются.
    return drawIcon(polygons.toPath(), color);
}

QIcon drawDrillIcon(QColor color) {
    QPainterPath painterPath;
    painterPath.addEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return drawIcon(painterPath, color);
}
