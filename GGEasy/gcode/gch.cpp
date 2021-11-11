// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
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
#include <QBoxLayout>
#include <QTextBrowser>
#include <ctre.hpp>

GCH::GCH(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // myClassFormat.setFontWeight(QFont::Bold);
}

void GCH::highlightBlock(const QString& text)
{
    //    qDebug(__FUNCTION__);

    static const mvector<char16_t> key { 'F', 'G', 'M', 'S', 'X', 'Y', 'Z' };
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
    static constexpr ctll::fixed_string pattern(R"(([GXYZFSM])([\+\-]?\d+\.?\d*))");
    auto data { toU16StrView(text) };
    for (auto [whole, code, number] : ctre::range<pattern>(data)) {
        { // code
            QTextCharFormat myClassFormat;
            switch (*code.data()) {
            case 'F':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0xFF, 0xFF, 0xFF));
                break;
            case 'G':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0xFF, 0xFF, 0x3F));
                break;
            case 'M':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0x7F, 0x7F, 0x7F));
                break;
            case 'S':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0xFF, 0x3F, 0xFF));
                break;
            case 'X':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0xFF, 0x3F, 0x3F));
                break;
            case 'Y':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0x3F, 0xFF, 0x3F));
                break;
            case 'Z':
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0x3F, 0x3F, 0xFF));
                break;
            default:
                myClassFormat.setFontWeight(QFont::Normal);
                myClassFormat.setForeground(QColor(0x7F, 0x7F, 0x7F));
                break;
            }
            //myClassFormat.setForeground(color[key.indexOf(*m.data())]);
            int start = std::distance(data.data(), code.data());
            int count = static_cast<int>(code.size());
            setFormat(start, count, myClassFormat);
        }
        { // number
            QTextCharFormat myClassFormat;
            myClassFormat.setFontWeight(QFont::Light);
            myClassFormat.setForeground(QColor(0x7F, 0x7F, 0x7F));
            //myClassFormat.setForeground(color[key.indexOf(*m.data())]);
            int start = std::distance(data.data(), number.data());
            int count = static_cast<int>(number.size());
            setFormat(start, count, myClassFormat);
        }
    }
}

Dialog::Dialog(const QString& text, const QString& windowTitle, QWidget* parent)
    : QDialog { parent }
{
    resize(600, 600);
    setWindowTitle(windowTitle);

    auto textBrowser = new QTextBrowser(this);
    textBrowser->setFontFamily("JetBrains Mono");
    textBrowser->setFontPointSize(16);
    new GCH(textBrowser->document());
    textBrowser->setPlainText(text);

    auto verticalLayout = new QVBoxLayout(this);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    verticalLayout->setContentsMargins(6, 6, 6, 6);
#else
    verticalLayout->setMargin(6);
#endif
    verticalLayout->addWidget(textBrowser);
}

Dialog::~Dialog() { }
