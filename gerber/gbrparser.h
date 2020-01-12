#ifndef GERBERPARSER_H
#define GERBERPARSER_H

#include "gbrfile.h"
#include "gbrtypes.h"
#include <QObject>
#include <QStack>
#include <parser.h>

namespace Gerber {

class Parser : public FileParser {
    Q_OBJECT
public:
    Parser(QObject* parent = nullptr);
    AbstractFile* parseFile(const QString& fileName) override;
    void parseLines(const QString& gerberLines, const QString& fileName);

private:
    QList<QString> cleanAndFormatFile(QString data);
    double arcAngle(double start, double stop);
    double toDouble(const QString& Str, bool scale = false, bool inchControl = true);
    bool parseNumber(QString Str, cInt& val, int integer, int decimal);

    void addPath();
    void addFlash();

    void reset(const QString& fileName);
    void resetStep();

    IntPoint parsePosition(const QString& xyStr);
    Path arc(const IntPoint& center, double radius, double start, double stop);
    Path arc(IntPoint p1, IntPoint p2, IntPoint center);

    Paths createLine();
    Paths createPolygon();

    ClipperLib::Clipper m_clipper;
    ClipperLib::ClipperOffset m_offset;

    QMap<QString, QString> m_apertureMacro;

    QStringList attributesStrings;
    QStringList apertureAttributesStrings;
    Component m_component;

    enum WorkingType {
        Normal,
        StepRepeat,
        ApertureBlock,
    };

    QStack<QPair<WorkingType, int>> m_abSrIdStack;

    Path m_path;
    State m_state;
    QString m_currentGerbLine;

    int m_lineNum = 0;
    int m_goId = 0;

    StepRepeatStr m_stepRepeat;

    bool parseAperture(const QString& gLine);
    bool parseApertureBlock(const QString& gLine);
    bool parseApertureMacros(const QString& gLine);
    bool parseAttributes(const QString& gLine);
    bool parseCircularInterpolation(const QString& gLine);
    bool parseDCode(const QString& gLine);
    bool parseEndOfFile(const QString& gLine);
    bool parseFormat(const QString& gLine);
    bool parseGCode(const QString& gLine);
    bool parseImagePolarity(const QString& gLine);
    bool parseLineInterpolation(const QString& gLine);
    bool parseStepRepeat(const QString& gLine);
    bool parseTransformations(const QString& gLine);
    bool parseUnitMode(const QString& gLine);
    void closeStepRepeat();

    inline File* file() { return reinterpret_cast<File*>(m_file); }

    /*inline*/ ApBlock* apBlock(int id) { return static_cast<ApBlock*>(file()->m_apertures[id].data()); }
};
}
#endif // GERBERPARSER_H
