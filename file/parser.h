#ifndef PARSER_H
#define PARSER_H

#include "abstractfile.h"
#include <QObject>

class File;

class FileParser : public QObject {
    Q_OBJECT
public:
    explicit FileParser(QObject* parent = nullptr);
    virtual ~FileParser();
    virtual AbstractFile* parseFile(const QString& fileName) = 0;

signals:
    void fileReady(AbstractFile* file);
    void fileProgress(const QString& fileName, int max, int value);
    void fileError(const QString& fileName, const QString& error);

protected:
    AbstractFile* m_file = nullptr;
};

#endif // PARSER_H
