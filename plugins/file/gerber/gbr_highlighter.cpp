// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author : Damir Bakiev *
 * Version : na *
 * Date : 11 November 2021*
 * Website : na *
 * Copyright : Damir Bakiev 2016-2021 *
 * License: *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt *
 *******************************************************************************/
#include "gbr_highlighter.h"
#include "mvector.h"
#include <ctre.hpp>

namespace Gerber {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    static QTextCharFormat myClassFormat;

    std::u16string_view data(reinterpret_cast<const char16_t*>(text.utf16()), text.size());

    for (auto m : ctre::range<R"(^%.+\*%$)">(data)) {
        myClassFormat.setForeground(QColor(0x80, 0x80, 0x00));
        int start = std::distance(data.data(), m.data());
        int count = static_cast<int>(m.size());
        setFormat(start, count, myClassFormat);
        return;
    }

    static const std::map<QChar, QColor> color {
        {'D', QColor(0x00, 0xFF, 0xFF)},
        {'G', QColor(0x7F, 0x7F, 0x7F)},
        {'I', QColor(0x00, 0x00, 0xFF)},
        {'J', QColor(0xFF, 0x00, 0xFF)},
        {'M', QColor(0x00, 0xFF, 0xFF)},
        {'X', QColor(0xFF, 0x00, 0x00)},
        {'Y', QColor(0x00, 0xFF, 0x00)},
    };

    for (auto m : ctre::range<R"([DGIJMXY][\+\-]?\d+\.?\d*)">(data)) {
        myClassFormat.setForeground(color.at(*m.data()));
        int start = std::distance(data.data(), m.data());
        int count = static_cast<int>(m.size());
        setFormat(start, count, myClassFormat);
    }
}

} // namespace Gerber
