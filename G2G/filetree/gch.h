#ifndef GCH_H
#define GCH_H

#include <QObject>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

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

#endif // GCH_H
