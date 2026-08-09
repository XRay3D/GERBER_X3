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
#pragma once

#include <QDialog>
#include <QSyntaxHighlighter>

class QTextDocument;
class QTextBrowser;

namespace GCode {

class Viewer3d;

// Просмотр УП: слева текст с подсветкой синтаксиса, справа 3D-вид траектории.
// Подсветка отрезков в 3D и выделение в тексте синхронизированы в обе стороны.
class Dialog : public QDialog {
    Q_OBJECT
public:
    Dialog(const QString& text, const QString& windowTitle, QWidget* parent = nullptr);
    ~Dialog() override;

    // Окно на файл ровно одно. Реестр держит сам Dialog, а не место вызова:
    // обновлять и закрывать его надо из File::regenerate и из деструктора узла,
    // и обоим неоткуда узнать про чужую статическую карту.
    static Dialog* showFor(int32_t fileId, const QString& text, const QString& windowTitle, QWidget* parent);
    // Текст УП пересобран -- показать новый, сохранив позицию курсора.
    static void programChanged(int32_t fileId, const QString& text, const QString& windowTitle);
    // Файл удалён -- смотреть больше нечего.
    static void programClosed(int32_t fileId);

    void setProgram(const QString& text, const QString& windowTitle);

private:
    int32_t fileId_{-1};
    QTextEdit* tbLine;
    QTextBrowser* tbCode;
    Viewer3d* viewer{}; // nullptr, если OpenGL недоступен

    void syncViewerFromText();
    void syncTextFromViewer(int lineNo);
    void updateExtraSelections();
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
