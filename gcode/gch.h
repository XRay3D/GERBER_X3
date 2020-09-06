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
