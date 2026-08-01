/*

This is a C++ port of the following PyQt example
http://diotavelli.net/PyQtWiki/Python%20syntax%20highlighting
C++ port by Frankie Simon (www.kickdrive.de, www.fuh-edv.de)

The following free software license applies for this file (u"X11 license"_s):

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the u"Software"_s), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED u"AS IS"_s, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

ChaiScriptSyntaxHighlighter.cpp

   Created on: 21.10.2016
       Author: klemens.morgenstern
 */

#include <widgets/ChaiScriptSyntaxHighlighter.hpp>

namespace widgets {

ChaiScriptSyntaxHighlighter::ChaiScriptSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    keywords = QStringList() << u"attr"_s
                             << u"auto"_s
                             << u"break"_s << // u"class"_s << u"def"_s <<
        u"else"_s
                             << u"for"_s
                             << u"fun"_s
                             << u"if"_s
                             << u"try"_s
                             << u"catch"_s
                             << u"while"_s
                             << u"var"_s
                             << u"true"_s
                             << u"false"_s;

    operators = QStringList() << u"="_s <<
        // Comparison
        u"=="_s
                              << u"!="_s
                              << u"<"_s
                              << u"<="_s
                              << u">"_s
                              << u">="_s <<
        // Arithmetic
        u"\\+"_s
                              << u"-"_s
                              << u"\\*"_s
                              << u"/"_s
                              << u"//"_s
                              << u"%"_s
                              << u"\\*\\*"_s <<
        // In-place
        u"\\+="_s
                              << u"-="_s
                              << u"\\*="_s
                              << u"/="_s
                              << u"%="_s <<
        // Bitwise
        u"\\^"_s
                              << u"\\|"_s
                              << u"&"_s
                              << u"~"_s
                              << u">>"_s
                              << u"<<"_s;

    braces = QStringList() << u"{"_s
                           << u"}"_s
                           << u"\\("_s
                           << u"\\)"_s
                           << u"\\["_s
                           << u"]"_s;

    basicStyles.insert(u"keyword"_s, getTextCharFormat(u"blue"_s));
    basicStyles.insert(u"operator"_s, getTextCharFormat(u"red"_s));
    basicStyles.insert(u"brace"_s, getTextCharFormat(u"darkGray"_s));
    basicStyles.insert(u"defclass"_s, getTextCharFormat(u"black"_s, u"bold"_s));
    basicStyles.insert(u"brace"_s, getTextCharFormat(u"darkGray"_s));
    basicStyles.insert(u"string"_s, getTextCharFormat(u"magenta"_s));
    basicStyles.insert(u"comment"_s, getTextCharFormat(u"darkGreen"_s, u"italic"_s));
    basicStyles.insert(u"numbers"_s, getTextCharFormat(u"brown"_s));

    initializeRules();
}

void ChaiScriptSyntaxHighlighter::initializeRules() {
    for(auto& currKeyword: keywords)
        rules.append(ChaiScriptRule(u"\\b%1\\b"_s.arg(currKeyword), 0, basicStyles.value(u"keyword"_s)));
    for(auto& currOperator: operators)
        rules.append(ChaiScriptRule(u"%1"_s.arg(currOperator), 0, basicStyles.value(u"operator"_s)));
    for(auto& currBrace: braces)
        rules.append(ChaiScriptRule(u"%1"_s.arg(currBrace), 0, basicStyles.value(u"brace"_s)));
    // Double-quoted string, possibly containing escape sequences
    // FF: originally in python : r'u"[^"_s\\]*(\\.[^u"\\]*)*"_s'
    rules.append(ChaiScriptRule(u"\"_s[^\u"\\\\]*(\\\\.[^\"_s\\\\]*)*\"u", 0, basicStyles.value("_sstring")));
    // Single-quoted string, possibly containing escape sequences
    // FF: originally in python : ru"'[^'\\]*(\\.[^'\\]*)*'"_s
    rules.append(ChaiScriptRule(u"'[^'\\\\]*(\\\\.[^'\\\\]*)*'"_s, 0, basicStyles.value(u"string"_s)));

    // 'def' followed by an identifier
    // FF: originally: r'\bdef\b\s*(\w+)'
    rules.append(ChaiScriptRule(u"\\bdef\\b\\s*(\\w+)"_s, 1, basicStyles.value(u"defclass"_s)));
    // 'class' followed by an identifier
    // FF: originally: r'\bclass\b\s*(\w+)'
    rules.append(ChaiScriptRule(u"\\bclass\\b\\s*(\\w+)"_s, 1, basicStyles.value(u"defclass"_s)));

    // From '#' until a newline
    // FF: originally: r'#[^\\n]*'
    rules.append(ChaiScriptRule(u"//[^\\n]*"_s, 0, basicStyles.value(u"comment"_s)));

    // Numeric literals
    rules.append(ChaiScriptRule(u"\\b[+-]?[0-9]+[lL]?\\b"_s, 0, basicStyles.value(u"numbers"_s)));                              // r'\b[+-]?[0-9]+[lL]?\b'
    rules.append(ChaiScriptRule(u"\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b"_s, 0, basicStyles.value(u"numbers"_s)));                   // r'\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\b'
    rules.append(ChaiScriptRule(u"\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b"_s, 0, basicStyles.value(u"numbers"_s))); // r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b'
}

void ChaiScriptSyntaxHighlighter::highlightBlock(const QString& text) {
    for(ChaiScriptRule& currRule: rules) {
        int idx = currRule.pattern.indexIn(text, 0);
        while(idx >= 0) {
            // Get index of Nth match
            idx = currRule.pattern.pos(currRule.nth);
            int length = currRule.pattern.cap(currRule.nth).length();
            setFormat(idx, length, currRule.format);
            idx = currRule.pattern.indexIn(text, idx + length);
        }
    }

    setCurrentBlockState(0);
}

bool ChaiScriptSyntaxHighlighter::matchMultiline(const QString& text, const QRegExp& delimiter, const int inState, const QTextCharFormat& style) {
    int start = -1;
    int add = -1;
    int end = -1;
    int length{};

    // If inside triple-single quotes, start at 0
    if(previousBlockState() == inState) {
        start = 0;
        add = 0;
    }
    // Otherwise, look for the delimiter on this line
    else {
        start = delimiter.indexIn(text);
        // Move past this match
        add = delimiter.matchedLength();
    }

    // As long as there's a delimiter match on this line...
    while(start >= 0) {
        // Look for the ending delimiter
        end = delimiter.indexIn(text, start + add);
        // Ending delimiter on this line?
        if(end >= add) {
            length = end - start + add + delimiter.matchedLength();
            setCurrentBlockState(0);
        }
        // No; multi-line string
        else {
            setCurrentBlockState(inState);
            length = text.length() - start + add;
        }
        // Apply formatting and look for next
        setFormat(start, length, style);
        start = delimiter.indexIn(text, start + length);
    }
    // Return True if still inside a multi-line string, False otherwise
    if(currentBlockState() == inState)
        return true;
    else
        return false;
}

const QTextCharFormat ChaiScriptSyntaxHighlighter::getTextCharFormat(const QString& colorName, const QString& style) {
    QTextCharFormat charFormat;
    QColor color{colorName};
    charFormat.setForeground(color);
    if(style.contains(u"bold"_s, Qt::CaseInsensitive))
        charFormat.setFontWeight(QFont::Bold);
    if(style.contains(u"italic"_s, Qt::CaseInsensitive))
        charFormat.setFontItalic(true);
    return charFormat;
}

} /* namespace widgets */
