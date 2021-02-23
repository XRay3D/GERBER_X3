// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "exh.h"
#include "mvector.h"
#include <ctre.hpp>

namespace Excellon {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    static QTextCharFormat myClassFormat;
    static const std::map<char, QColor> color {
        { 'A', QColor(0x00, 0x00, 0xFF) },
        { 'C', QColor(0xFF, 0xFF, 0x00) },
        { 'G', QColor(0xFF, 0xFF, 0xFF) },
        { 'M', QColor(0xFF, 0x00, 0xFF) },
        { 'T', QColor(0x00, 0xFF, 0xFF) },
        { 'X', QColor(0xFF, 0x00, 0x00) },
        { 'Y', QColor(0x00, 0xFF, 0x00) },
    };

    using namespace std::string_view_literals;
    static constexpr auto pattern = ctll::fixed_string("([ACGMTXY])([\\+\\-]?\\d+\\.?\\d*)");
    auto data = reinterpret_cast<const char16_t*>(text.data());
    for (auto m : ctre::range<pattern>(text)) {
        myClassFormat.setForeground(color.at(*m.data()));
        setFormat(std::distance(data, m.data()), m.size(), myClassFormat);
    }
}

}
