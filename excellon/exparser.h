#ifndef EXPARSER_H
#define EXPARSER_H

#include "exvars.h"
#include <QObject>
#include <parser.h>

namespace Excellon {

class Parser : public FileParser {
    Q_OBJECT

public:
    explicit Parser(QObject* parent = nullptr);
    AbstractFile* parseFile(const QString& fileName) override;
    bool isDrillFile(const QString& fileName);
    static double parseNumber(QString Str, const State& state);

private:
    bool parseComment(const QString& line);
    bool parseGCode(const QString& line);
    bool parseMCode(const QString& line);
    bool parseTCode(const QString& line);
    bool parsePos(const QString& line);
    bool parseSlot(const QString& line);
    bool parseRepeat(const QString& line);
    bool parseFormat(const QString& line);
    bool parseNumber(QString Str, double& val);

    inline File* file() { return reinterpret_cast<File*>(m_file); }

    State m_state;
};
} // namespace Excellon

#endif // EXPARSER_H
