/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <QDialog>
#include <QSyntaxHighlighter>

class QTextDocument;

namespace GCode {

class Dialog : public QDialog {
    Q_OBJECT
public:
    Dialog(const QString& text, const QString& windowTitle, QWidget* parent = nullptr);
    ~Dialog() override = default;
};

class Highlighter final : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument* parent);

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString& text) override final;
};

} // namespace GCode
