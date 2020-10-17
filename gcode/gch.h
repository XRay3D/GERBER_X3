/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QSyntaxHighlighter>

class QTextDocument;

class GCH : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit GCH(QTextDocument* parent);

signals:

    // QSyntaxHighlighter interface
protected:
    void highlightBlock(const QString& text) override;

private:
};
