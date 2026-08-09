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
#include "gc_programdialog.h"
#include "gc_viewer3d.h"

#include "app.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
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
            case 'X': txtChF.setForeground(axisColor[0]); break;
            case 'Y': txtChF.setForeground(axisColor[1]); break;
            case 'Z': txtChF.setForeground(axisColor[2]); break;
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

namespace {
// Открытые окна просмотра, по одному на файл.
std::map<int32_t, Dialog*>& openDialogs() {
    static std::map<int32_t, Dialog*> dialogs;
    return dialogs;
}
} // namespace

Dialog* Dialog::showFor(int32_t fileId, const QString& text, const QString& windowTitle, QWidget* parent) {
    auto& dialogs = openDialogs();
    if(auto it = dialogs.find(fileId); it != dialogs.end()) {
        it->second->setProgram(text, windowTitle);
        it->second->raise();
        it->second->activateWindow();
        return it->second;
    }
    auto* dialog = new Dialog{text, windowTitle, parent};
    dialog->fileId_ = fileId;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialogs.emplace(fileId, dialog);
    dialog->show();
    return dialog;
}

void Dialog::programChanged(int32_t fileId, const QString& text, const QString& windowTitle) {
    auto& dialogs = openDialogs();
    if(auto it = dialogs.find(fileId); it != dialogs.end())
        it->second->setProgram(text, windowTitle);
}

void Dialog::programClosed(int32_t fileId) {
    auto& dialogs = openDialogs();
    if(auto it = dialogs.find(fileId); it != dialogs.end()) {
        Dialog* dialog = it->second;
        // Из реестра убираем СРАЗУ: close() при WA_DeleteOnClose удаляет окно
        // отложенно, и до этого момента id успел бы достаться новому файлу --
        // тот получил бы уже закрывающееся окно.
        dialogs.erase(it);
        dialog->fileId_ = -1; // деструктору искать больше нечего
        dialog->close();
    }
}

Dialog::~Dialog() {
    if(fileId_ > -1) openDialogs().erase(fileId_);
}

void Dialog::setProgram(const QString& text, const QString& windowTitle) {
    setWindowTitle(windowTitle);

    // Позиция курсора сохраняется: при перегенерации текст обычно тот же по
    // смыслу, и терять место, на которое человек смотрел, незачем.
    const int cursorPos = tbCode->textCursor().position();

    tbCode->setPlainText(text);

    const int lineCount = text.count(u'\n') + 3;
    tbLine->setPlainText(v::iota(1, lineCount)
        | v::transform(std::bind(qOverload<int, int>(QString::number), _1, 10))
        | v::join_with(u'\n')
        | r::to<QString>());
    tbLine->setFixedWidth(QFontMetrics{tbLine->font()}.boundingRect(QString::number(lineCount * 10)).width());

    if(QTextCursor cursor = tbCode->textCursor(); cursorPos > 0) {
        cursor.setPosition(std::min(cursorPos, tbCode->document()->characterCount() - 1));
        tbCode->setTextCursor(cursor);
    }

    viewer->setProgramText(text);
    syncViewerFromText();
}

Dialog::Dialog(const QString& text, const QString& windowTitle, QWidget* parent)
    : QDialog{parent} {
    resize(1200, 700);
    setWindowTitle(windowTitle);

    QFont font{
        {u"JetBrains Mono"_s, u"Iosevka Extended"_s, u"Monospace"_s}
    };
    font.setPointSize(16);

    auto textPane = new QWidget{this};
    tbCode = new QTextBrowser{textPane};
    tbCode->setFont(font);
    tbCode->setWordWrapMode({});
    // Курсор должен ходить и с клавиатуры — иначе не отследить текущую строку.
    tbCode->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    new Highlighter{tbCode->document()};

    tbLine = new QTextEdit{textPane};
    tbLine->setFont(font);
    tbLine->setReadOnly(true);
    tbLine->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(
        tbCode->verticalScrollBar(), &QScrollBar::valueChanged,
        tbLine->verticalScrollBar(), &QScrollBar::setValue);

    {
        auto textLayout = new QHBoxLayout{textPane};
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(0);
        textLayout->addWidget(tbLine);
        textLayout->addWidget(tbCode);
    }

    auto viewPane = new QWidget{this};
    viewer = new Viewer3d{viewPane};
    {
        auto viewLayout = new QVBoxLayout{viewPane};
        viewLayout->setContentsMargins(0, 0, 0, 0);
        viewLayout->setSpacing(4);

        auto toolLayout = new QHBoxLayout;
        toolLayout->setContentsMargins(0, 0, 0, 0);
        auto addButton = [&](const QString& title, Viewer3d::ViewPreset preset) {
            auto btn = new QPushButton{title, viewPane};
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, viewer, [this, preset] { viewer->setViewPreset(preset); });
            toolLayout->addWidget(btn);
        };
        auto btnFit = new QPushButton{tr("Fit"), viewPane};
        btnFit->setFocusPolicy(Qt::NoFocus);
        connect(btnFit, &QPushButton::clicked, viewer, [this] { viewer->fitToView(); });
        toolLayout->addWidget(btnFit);
        addButton(tr("Iso"), Viewer3d::Isometric);
        addButton(tr("Top"), Viewer3d::Top);
        addButton(tr("Front"), Viewer3d::Front);
        addButton(tr("Left"), Viewer3d::Left);

        auto chbxRapids = new QCheckBox{tr("Rapids"), viewPane};
        chbxRapids->setChecked(viewer->rapidsVisible());
        chbxRapids->setFocusPolicy(Qt::NoFocus);
        connect(chbxRapids, &QCheckBox::toggled, viewer, [this](bool checked) { viewer->setRapidsVisible(checked); });
        toolLayout->addWidget(chbxRapids);

        auto chbxPerspective = new QCheckBox{tr("Perspective"), viewPane};
        chbxPerspective->setChecked(viewer->perspective()); // состояние из настроек
        chbxPerspective->setFocusPolicy(Qt::NoFocus);
        connect(chbxPerspective, &QCheckBox::toggled, viewer, [this](bool checked) { viewer->setPerspective(checked); });
        toolLayout->addWidget(chbxPerspective);
        toolLayout->addStretch();

        viewLayout->addLayout(toolLayout);
        viewLayout->addWidget(viewer);
    }

    setProgram(text, windowTitle);

    // Текст -> 3D и 3D -> текст.
    connect(tbCode, &QTextEdit::cursorPositionChanged, this, &Dialog::syncViewerFromText);
    connect(tbCode, &QTextEdit::selectionChanged, this, &Dialog::syncViewerFromText);
    connect(viewer, &Viewer3d::lineSelected, this, &Dialog::syncTextFromViewer);

    auto splitter = new QSplitter{Qt::Horizontal, this};
    splitter->addWidget(textPane);
    splitter->addWidget(viewPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    auto layout = new QHBoxLayout{this};
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(0);
    layout->addWidget(splitter);

    syncViewerFromText();
}

void Dialog::syncViewerFromText() {
    const QTextCursor cursor = tbCode->textCursor();
    const auto* doc = tbCode->document();
    viewer->setHighlightedLines(
        doc->findBlock(cursor.selectionStart()).blockNumber(),
        doc->findBlock(cursor.selectionEnd()).blockNumber());
    updateExtraSelections();
}

void Dialog::syncTextFromViewer(int lineNo) {
    const QTextBlock block = tbCode->document()->findBlockByNumber(lineNo);
    if(!block.isValid()) return;
    QTextCursor cursor{block};
    cursor.select(QTextCursor::LineUnderCursor);
    // Обратная синхронизация 3D -> текст замыкается на cursorPositionChanged,
    // который вернёт подсветку в 3D уже для этой строки.
    tbCode->setTextCursor(cursor);
    tbCode->ensureCursorVisible();
}

void Dialog::updateExtraSelections() {
    // Подсветка выбранных строк тем же цветом, что и отрезки в 3D.
    QColor color = App::settings().guiColor(GuiColors::Pin);
    color.setAlpha(std::min(color.alpha(), 96));

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(color);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = tbCode->textCursor();
    selection.cursor.clearSelection();
    tbCode->setExtraSelections({selection});
}

} // namespace GCode

#include "moc_gc_programdialog.cpp"
