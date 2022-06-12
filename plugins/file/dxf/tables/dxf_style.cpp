// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_style.h"
#include "dxf_file.h"
#include "settings.h"
#include <QFontDatabase>

namespace Dxf {

Style::Style(SectionParser* sp)
    : AbstractTable(sp) {
}

void Style::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch (code.code()) {
        case SubclassMarker: // 100
            break;
        case StyleName: // 2
            styleName = code.string();
            sp->file->styles()[styleName] = this;
            break;
        case StandardFlag: // 70
            standardFlag = code;
            break;
        case FixedTextHeight: // 40
            fixedTextHeight = code;
            break;
        case WidthFactor: // 41
            break;
        case ObliqueAngle: // 50
            break;
        case TextGenerationFlag: // 71
            textGenerationFlag = code;
            break;
        case LastHeightUsed: // 42
            break;
        case PrimaryFontFileName: // 3
            break;
        case BigfontFileName: // 4
            break;
        case ALongValueWhichContainsATruetypeFontsPitchAndFamily_CharacterSet_AndItalicAndBoldFlags: // 1071
            if ((int32_t(code) & 0xA) == 0xA) {
                font.setBold(false);
                font.setItalic(false);
            }
            if ((int32_t(code) & 0x1000020) == 0x1000020)
                font.setBold(true);
            if ((int32_t(code) & 0x2000030) == 0x2000030)
                font.setItalic(true);
            break;
        case FontFamily: // 1000
            font.setPointSize(100);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            for (auto& family : QFontDatabase().families()) {
#else
            for (auto& family : QFontDatabase::families()) {
#endif
                if (family.contains(code.string(), Qt::CaseInsensitive)) {
                    font.setFamily(code.string());
                    break;
                }
            }
            if (font.family() != code) {
                qDebug() << font.family();
                font.setFamily(Settings::defaultFont());
            }
            break;
        default:
            AbstractTable::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
    //    qDebug() << data.size();
    //    for (auto& code : data)
    //        qDebug() << "\t" << code;
}

} // namespace Dxf
