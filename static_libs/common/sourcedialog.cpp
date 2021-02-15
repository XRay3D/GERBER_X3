#include "sourcedialog.h"

#include <interfaces/file.h>

#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QVBoxLayout>

SourceDialog::SourceDialog(FileInterface* file, QSyntaxHighlighter* hl, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QString::fromUtf8("SourceDialog"));
    auto* textBrowser = new QTextBrowser(this);
    textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
    textBrowser->setFontFamily("JetBrains Mono");
    textBrowser->setLineWrapMode(QTextEdit::NoWrap);
    hl->setParent(textBrowser);
    hl->setDocument(textBrowser->document());
    auto* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    verticalLayout->setMargin(6);
#else
    verticalLayout->setContentsMargins(6, 6, 6, 6);
#endif
    verticalLayout->addWidget(textBrowser);
    QString s;
    s.reserve(1000000);
    for (const QString& str : file->lines())
        s += str + '\n';
    textBrowser->setPlainText(s);
}
