// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
* *
* Author : Damir Bakiev *
* Version : na *
* Date : 14 January 2021 *
* Website : na *
* Copyright : Damir Bakiev 2016-2021 *
* *
* License: *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt *
* *
*******************************************************************************/
#include "gbrh.h"
#include "mvector.h"
#include <ctre.hpp>

namespace Gerber {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    auto data = reinterpret_cast<const char16_t*>(text.data());

    static QTextCharFormat myClassFormat;
    //myClassFormat.setFontWeight(QFont::Bold);

    static constexpr auto pattern2 = ctll::fixed_string("^%.+\\*%$");
    for (auto m : ctre::range<pattern2>(data)) {
        myClassFormat.setForeground(Qt::darkMagenta);
        setFormat(std::distance(data, m.data()), static_cast<int>(m.size()), myClassFormat);
    }

    static const mvector<QColor> color {
        /*  D  */ QColor(255, 0, 0),
        /*  G  */ QColor(0, 0, 0),
        /*  I  */ QColor(0, 0, 127),
        /*  J  */ QColor(127, 0, 127),
        /*  M  */ QColor(0, 127, 127),
        /*  X  */ QColor(127, 0, 0),
        /*  Y  */ QColor(0, 127, 0),
    };

    using namespace std::string_view_literals;
    static constexpr auto pattern = ctll::fixed_string("([DGIJMXY])([\\+\\-]?\\d+\\.?\\d*)");
    for (auto m : ctre::range<pattern>(data)) {
        static const mvector key { 'D', 'G', 'I', 'J', 'M', 'X', 'Y' };
        myClassFormat.setForeground(color[key.indexOf(*m.data())]);
        setFormat(std::distance(data, m.data()), static_cast<int>(m.size()), myClassFormat);
    }
}
}
