/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_highlighter.h"

#include <QBoxLayout>
#include <QScrollBar>
#include <QTextBrowser>
#include <ctre.hpp>

using namespace Qt::Literals;

using namespace std::placeholders;

namespace r = std ::ranges;
namespace v = std ::views;

namespace GCode {

Highlighter::Highlighter(QTextDocument* parent)
    : QSyntaxHighlighter{parent} {
    // txtChF.setFontWeight(QFont::Bold);
}

void Highlighter::highlightBlock(const QString& text) {
    static constexpr ctll::fixed_string pattern{R"(([FGIJMSXYZ])([\+\-]?\d+\.?\d*))"};
    std::u16string_view data{text};
    for(auto [whole, code, number]: ctre::search_all<pattern>(data)) {
        { // code
            QTextCharFormat txtChF;
            txtChF.setFontWeight(QFont::Normal);
            switch(*code.data()) {
            case 'F': txtChF.setForeground(QColor{0xFF, 0xFF, 0xFF}); break;
            case 'G': txtChF.setForeground(QColor{0xFF, 0xFF, 0x3F}); break;
            case 'I': txtChF.setForeground(QColor{0xFF, 0x3F, 0xFF}); break;
            case 'J': txtChF.setForeground(QColor{0x3F, 0x3F, 0xFF}); break;
            case 'M': txtChF.setForeground(QColor{0x7F, 0x7F, 0x7F}); break;
            case 'S': txtChF.setForeground(QColor{0xFF, 0x3F, 0xFF}); break;
            case 'X': txtChF.setForeground(QColor{0xFF, 0x3F, 0x3F}); break;
            case 'Y': txtChF.setForeground(QColor{0x3F, 0xFF, 0x3F}); break;
            case 'Z': txtChF.setForeground(QColor{0x3F, 0x3F, 0xFF}); break;
            default : txtChF.setForeground(QColor{0x7F, 0x7F, 0x7F}); break;
            }
            // txtChF.setForeground(color[key.indexOf(*m.data())]);
            int start = std::distance(data.data(), code.data());
            setFormat(start, /*count*/ code.size(), txtChF);
        }
        { // number
            QTextCharFormat txtChF;
            txtChF.setFontWeight(QFont::Light);
            txtChF.setForeground(QColor{0x7F, 0x7F, 0x7F});
            // txtChF.setForeground(color[key.indexOf(*m.data())]);
            int start = std::distance(data.data(), number.data());
            setFormat(start, /*count*/ number.size(), txtChF);
        }
    }
}

Dialog::Dialog(const QString& text, const QString& windowTitle, QWidget* parent)
    : QDialog{parent} {
    resize(600, 600);
    setWindowTitle(windowTitle);
    auto tbCode = new QTextBrowser{this};
    tbCode->setFontFamily(u"JetBrains Mono"_s);
    tbCode->setFontPointSize(16);
    tbCode->setWordWrapMode({});
    new Highlighter{tbCode->document()};
    tbCode->setPlainText(text);

    int lineCount = text.count(u'\n') + 3;

    auto tbLine = new QTextEdit{this};
    tbLine->setFontFamily(u"JetBrains Mono"_s);
    tbLine->setFontPointSize(16);
    tbLine->setReadOnly(true);
    tbLine->setPlainText(v::iota(1, lineCount)
        | v::transform(std::bind(qOverload<int, int>(QString::number), _1, 10))
        | v::join_with(u'\n')
        | r::to<QString>());
    tbLine->setFixedWidth(
        tbLine->fontMetrics()
            .boundingRect(QString::number(lineCount))
            .width()
        * 3);

    tbLine->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(
        tbCode->verticalScrollBar(), &QScrollBar::valueChanged,
        tbLine->verticalScrollBar(), &QScrollBar::setValue);

    auto layout = new QHBoxLayout{this};
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(0);
    layout->addWidget(tbLine);
    layout->addWidget(tbCode);
}

} // namespace GCode

#include "moc_gc_highlighter.cpp"
