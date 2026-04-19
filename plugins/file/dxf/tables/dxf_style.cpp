/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_style.h"
#include "dxf_file.h"
#include "settings.h"
#include <QFontDatabase>

namespace Dxf {

Style::Style(SectionParser* sp)
    : AbstractTable{sp} {
}

void Style::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        case SubclassMarker: break; // 100
        case StyleName:             // 2
            styleName = code.string();
            sp->file->styles()[styleName] = this;
            break;
        case StandardFlag       : standardFlag = code; break;       // 70
        case FixedTextHeight    : fixedTextHeight = code; break;    // 40
        case WidthFactor        : break;                            // 41
        case ObliqueAngle       : break;                            // 50
        case TextGenerationFlag : textGenerationFlag = code; break; // 71
        case LastHeightUsed     : break;                            // 42
        case PrimaryFontFileName: break;                            // 3
        case BigfontFileName    : break;                                // 4

        case ALongValueWhichContainsATruetypeFontsPitchAndFamily_CharacterSet_AndItalicAndBoldFlags: // 1071
            if((int32_t(code) & 0xA) == 0xA) {
                font.setBold(false);
                font.setItalic(false);
            }
            if((int32_t(code) & 0x1000020) == 0x1000020)
                font.setBold(true);
            if((int32_t(code) & 0x2000030) == 0x2000030)
                font.setItalic(true);
            break;
        case FontFamily: // 1000
            font.setPointSize(100);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            for(auto& family: QFontDatabase().families()) {
#else
            for(auto& family: QFontDatabase::families()) {
#endif
                if(family.contains(code.string(), Qt::CaseInsensitive)) {
                    font.setFamily(code.string());
                    break;
                }
            }
            if(font.family() != code) {
                qDebug() << font.family();
                font.setFamily(Settings::defaultFont());
            }
            break;
        default: AbstractTable::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);

    // for (auto& code : data)
}

} // namespace Dxf
