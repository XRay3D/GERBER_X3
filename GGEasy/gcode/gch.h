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
