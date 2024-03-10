// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

    if(text.startsWith("%") && text.endsWith("*%")) {
        myClassFormat.setForeground(QColor(0x80, 0x80, 0x00));
        setFormat(0, text.size(), myClassFormat);
        return;
    }

    static constexpr auto color = [](QChar c) -> QColor {
        switch(c.toLatin1()) {
        case 'D': return {0x00, 0xFF, 0xFF};
        case 'G': return {0x7F, 0x7F, 0x7F};
        case 'I': return {0x00, 0x00, 0xFF};
        case 'J': return {0xFF, 0x00, 0xFF};
        case 'M': return {0x00, 0xFF, 0xFF};
        case 'X': return {0xFF, 0x00, 0x00};
        case 'Y': return {0x00, 0xFF, 0x00};
        default: return {};
        }
    };

    std::u16string_view data(reinterpret_cast<const char16_t*>(text.utf16()), text.size());

    for(auto&& m: ctre::search_all<R"([DGIJMXY][+\-]?\d+\.?\d*)">(data)) {
        myClassFormat.setForeground(color(*m.data()));
        int start = std::distance(data.data(), m.data());
        int count = static_cast<int>(m.size());
        setFormat(start, count, myClassFormat);
    }
}

} // namespace Gerber

#include "moc_gbr_highlighter.cpp"
