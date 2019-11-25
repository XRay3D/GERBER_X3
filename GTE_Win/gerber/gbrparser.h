#ifndef GERBERPARSER_H
#define GERBERPARSER_H

#include "gbraperture.h"
#include "gbrvars.h"
#include <QObject>
#include <QStack>

namespace Gerber {

class Parser : public QObject {
    Q_OBJECT
public:
    Parser(QObject* parent = nullptr);
    Paths parseFile(const QString& fileName);
    Paths parseLines(const QString& gerberLines);

private:
    QList<QString> format(QString data);
    double arcAngle(double start, double stop);
    double toDouble(const QString& Str, bool scale = false, bool inchControl = true);
    bool parseNumber(QString Str, cInt& val, int integer, int decimal);

    void addPath();
    void addFlash();

    void reset();
    void resetStep();

    IntPoint parsePosition(const QString& xyStr);
    Path arc(const IntPoint& center, double radius, double start, double stop);
    Path arc(IntPoint p1, IntPoint p2, IntPoint center);

    Paths createLine();
    Paths createPolygon();

    ClipperLib::Clipper m_clipper;
    ClipperLib::ClipperOffset m_offset;

    QMap<QString, QString> m_apertureMacro;

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

    QMap<int, QSharedPointer<AbstractAperture>> m_apertures;
    ApBlock* apBlock(int id) { return static_cast<ApBlock*>(m_apertures[id].data()); }
    QList<QString> m_lines;
    Paths m_mergedPaths;
    QList<GraphicObject> m_file;
    Format m_format;
    QVector<int> rawIndex;

    Paths merge()
    {
        m_mergedPaths.clear();
        int i = 0;
        while (i < m_file.size()) {
            Clipper clipper;
            clipper.AddPaths(m_mergedPaths, ptSubject, true);
            const auto exp = m_file.at(i).state().imgPolarity();
            do {
                const GraphicObject& go = m_file.at(i++);
                clipper.AddPaths(go.paths(), ptClip, true);
            } while (i < m_file.size() && exp == m_file.at(i).state().imgPolarity());
            if (m_file.at(i - 1).state().imgPolarity() == Positive)
                clipper.Execute(ctUnion, m_mergedPaths, pftPositive);
            else
                clipper.Execute(ctDifference, m_mergedPaths, pftNonZero);
        }
        return m_mergedPaths;
    }
};
}
#endif // GERBERPARSER_H
