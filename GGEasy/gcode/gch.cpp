// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "gch.h"
#include "mvector.h"
#include <ctre.hpp>

GCH::GCH(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void GCH::highlightBlock(const QString& text)
{
    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Bold);
    static const mvector<Qt::GlobalColor> color {
        Qt::darkMagenta, // 'F',
        Qt::black, //       'G',
        Qt::darkYellow, //  'M',
        Qt::gray, //        'S',
        Qt::red, //         'X',
        Qt::darkGreen, //   'Y',
        Qt::blue, //        'Z',
    };

    using namespace std::string_view_literals;
    static constexpr auto pattern = ctll::fixed_string("([GXYZFSM])([\\+\\-]?\\d+\\.?\\d*)");
    auto data = reinterpret_cast<const char16_t*>(text.data());
    for (auto m : ctre::range<pattern>(data)) {
        static const mvector<char16_t> key { 'F', 'G', 'M', 'S', 'X', 'Y', 'Z' };
        myClassFormat.setForeground(color[key.indexOf(*m.data())]);
        setFormat(std::distance(data, m.data()), m.size(), myClassFormat);
    }
}
