// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gch.h"
#include <QRegularExpression>

GCH::GCH(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void GCH::highlightBlock(const QString& text)
{
    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Bold);
    QRegularExpression expression("([GXYZFSM])([+-]?\\d+\\.?\\d*)");
    //  QRegularExpression expression("\\bMy[A-Za-z]+\\b");
    QRegularExpressionMatchIterator i = expression.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();


        static const QVector<QChar> key {
            'F',
            'G',
            'M',
            'S',
            'X',
            'Y',
            'Z',
        };
        static const QVector<Qt::GlobalColor> color {
            Qt::darkMagenta, // 'F',
            Qt::black, //       'G',
            Qt::darkYellow, //  'M',
            Qt::gray, //        'S',
            Qt::red, //         'X',
            Qt::darkGreen, //   'Y',
            Qt::blue, //        'Z',
        };
        myClassFormat.setForeground(color.value(key.indexOf(match.captured(1).front().toUpper())));
        setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
    }
}
