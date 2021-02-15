#pragma once

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QDialog>
#else
#include <QtGui/QWidget>
#endif

class FileInterface;
class QSyntaxHighlighter;

class SourceDialog : public QDialog {
    Q_OBJECT
public:
    explicit SourceDialog(FileInterface* file, QSyntaxHighlighter* hl, QWidget* parent = nullptr);
};
