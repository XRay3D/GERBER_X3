//// This is an open source non-commercial project. Dear PVS-Studio, please check it.
//// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  March 25, 2023                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2023                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// *******************************************************************************/
// #include "topor_highlighter.h"
// #include "mvector.h"
// #include <ctre.hpp>

// namespace TopoR {

// SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
//     : QSyntaxHighlighter(parent) {
// }

// void SyntaxHighlighter::highlightBlock(const QString& text) {
//     static QTextCharFormat myClassFormat;
//     static const std::map<char, QColor> color {
//         {'A', QColor(0x00, 0x00, 0xFF)},
//         {'C', QColor(0xFF, 0xFF, 0x00)},
//         {'G', QColor(0xFF, 0xFF, 0xFF)},
//         {'M', QColor(0xFF, 0x00, 0xFF)},
//         {'T', QColor(0x00, 0xFF, 0xFF)},
//         {'X', QColor(0xFF, 0x00, 0x00)},
//         {'Y', QColor(0x00, 0xFF, 0x00)},
//     };

//    std::u16string_view data(reinterpret_cast<const char16_t*>(text.utf16()), text.size());
//    static constexpr ctll::fixed_string pattern(R"(([ACGMTXY])([\+\-]?\d+\.?\d*))");
//    for (auto m : ctre::range<pattern>(data)) {
//        myClassFormat.setForeground(color.at(*m.data()));
//        int start = std::distance(data.data(), m.data());
//        int count = static_cast<int>(m.size());
//        setFormat(start, count, myClassFormat);
//    }
//}

//} // namespace TopoR

// #include "moc_file_highlighter.cpp"
