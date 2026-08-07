/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <QDialog>
#include <QSyntaxHighlighter>

class QPlainTextEdit;
class QLabel;

namespace Gerber2 {

class File;

// Простая подсветка команд Gerber.
class Highlighter : public QSyntaxHighlighter {
public:
    explicit Highlighter(QTextDocument* doc);

protected:
    void highlightBlock(const QString& text) override;
};

// Редактор исходного текста Gerber с применением и сохранением.
class Editor : public QDialog {
    Q_OBJECT

public:
    explicit Editor(File* file, QWidget* parent = nullptr);

private:
    void apply();
    void save();
    void saveAs();
    void setStatus(const QString& text, bool error);

    File* file_;
    QPlainTextEdit* edit_;
    QLabel* status_;
};

} // namespace Gerber2
