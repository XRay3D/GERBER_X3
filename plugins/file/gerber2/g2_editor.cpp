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
#include "g2_editor.h"
#include "g2_file.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

namespace Gerber2 {

namespace {

struct Rule {
    QRegularExpression re;
    QTextCharFormat fmt;
};

const std::vector<Rule>& rules() {
    static const std::vector<Rule> value = [] {
        auto make = [](QStringView pattern, QColor color, bool bold = false) {
            QTextCharFormat f;
            f.setForeground(color);
            if(bold) f.setFontWeight(QFont::Bold);
            return Rule{QRegularExpression{pattern.toString()}, f};
        };
        return std::vector<Rule>{
            make(uR"(G04[^*]*)", Qt::darkGreen),                    // комментарий
            make(uR"(%[A-Z]{2})", QColor{0x80, 0x00, 0x80}, true),  // расширенная команда
            make(uR"(\b[DGM]\d{2,}\b)", QColor{0x00, 0x00, 0xC0}),  // коды
            make(uR"([XYIJ][+-]?\d+)", QColor{0x80, 0x40, 0x00}),   // координаты
            make(uR"([*%])", Qt::darkGray),                         // разделители
        };
    }();
    return value;
}

} // namespace

Highlighter::Highlighter(QTextDocument* doc)
    : QSyntaxHighlighter{doc} { }

void Highlighter::highlightBlock(const QString& text) {
    for(const Rule& rule: rules())
        for(auto&& m: rule.re.globalMatch(text))
            setFormat(m.capturedStart(), m.capturedLength(), rule.fmt);
}

// -----------------------------------------------------------------------------

Editor::Editor(File* file, QWidget* parent)
    : QDialog{parent}
    , file_{file} {
    setWindowTitle(tr("Gerber source — %1").arg(file->shortName()));
    resize(900, 700);

    edit_ = new QPlainTextEdit{this};
    edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    edit_->setLineWrapMode(QPlainTextEdit::NoWrap);
    edit_->setPlainText(file->source());
    new Highlighter{edit_->document()};

    status_ = new QLabel{this};
    status_->setWordWrap(true);

    auto* buttons = new QDialogButtonBox{this};
    auto* applyBtn = buttons->addButton(tr("&Apply"), QDialogButtonBox::ApplyRole);
    auto* saveBtn = buttons->addButton(tr("&Save"), QDialogButtonBox::ActionRole);
    auto* saveAsBtn = buttons->addButton(tr("Save &As…"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);

    connect(applyBtn, &QPushButton::clicked, this, &Editor::apply);
    connect(saveBtn, &QPushButton::clicked, this, &Editor::save);
    connect(saveAsBtn, &QPushButton::clicked, this, &Editor::saveAs);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(edit_);
    layout->addWidget(status_);
    layout->addWidget(buttons);

    if(const auto& warnings = file->parsed().warnings; !warnings.isEmpty())
        setStatus(warnings.join(u"; "_s), false);
}

void Editor::setStatus(const QString& text, bool error) {
    status_->setText(text);
    status_->setStyleSheet(error ? u"color: red"_s : u"color: gray"_s);
}

void Editor::apply() {
    QString error;
    if(!file_->setSource(edit_->toPlainText(), &error))
        return setStatus(tr("Parse error: %1").arg(error), true);
    const auto& warnings = file_->parsed().warnings;
    setStatus(warnings.isEmpty() ? tr("Applied.") : warnings.join(u"; "_s), false);
}

void Editor::save() {
    apply();
    QString error;
    if(!file_->saveAs(file_->name(), &error))
        return setStatus(tr("Save error: %1").arg(error), true);
    setStatus(tr("Saved to %1").arg(file_->name()), false);
}

void Editor::saveAs() {
    QString name = QFileDialog::getSaveFileName(this, tr("Save Gerber File"),
        file_->name(), tr("Gerber (*.gbr *.GBR)"));
    if(name.isEmpty()) return;
    apply();
    QString error;
    if(!file_->saveAs(name, &error))
        return setStatus(tr("Save error: %1").arg(error), true);
    setStatus(tr("Saved to %1").arg(name), false);
}

} // namespace Gerber2

#include "moc_g2_editor.cpp"
