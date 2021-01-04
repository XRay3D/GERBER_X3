/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "app.h"
#include "gbrattributes.h"
#include "gbrfile.h"
#include "gbrtypes.h"

#include "interfaces/parser.h"
#include "settings.h"

#include <QObject>
#include <QStack>

namespace Gerber {

class Parser : public QObject, public ParserInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "gerber.json")
    Q_INTERFACES(ParserInterface)

public:
    Parser(QObject* parent = nullptr);

    bool thisIsIt(const QString& fileName) override;
    QObject* getObject() override;
    int type() const override;
    NodeInterface* createNode(FileInterface* file) override;
    std::shared_ptr<FileInterface> createFile() override;
    void setupInterface(App* a, AppSettings* s) override;
    void createMainMenu(QMenu& menu, FileTreeView* tv) override;

public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

signals:
    void fileReady(FileInterface* file) override;
    void fileProgress(const QString& fileName, int max, int value) override;
    void fileError(const QString& fileName, const QString& error) override;

private:
    void parseLines(const QString& gerberLines, const QString& fileName);

    QVector<QString> cleanAndFormatFile(QString data);
    double arcAngle(double start, double stop);
    double toDouble(const QString& Str, bool scale = false, bool inchControl = true);
    bool parseNumber(QString Str, cInt& val, int integer, int decimal);

    void addPath();
    void addFlash();

    void reset(const QString& fileName);
    void resetStep();

    Point64 parsePosition(const QString& xyStr);
    Path arc(const Point64& center, double radius, double start, double stop);
    Path arc(Point64 p1, Point64 p2, Point64 center);

    Paths createLine();
    Paths createPolygon();

    ClipperLib::Clipper m_clipper;
    ClipperLib::ClipperOffset m_offset;

    QMap<QString, QString> m_apertureMacro;

    struct WorkingType {
        enum eWT {
            Normal,
            StepRepeat,
            ApertureBlock,
        };
        eWT workingType = Normal;
        int apertureBlockId = 0;
    };

    QStack<WorkingType> m_abSrIdStack;

    Path m_path;
    State m_state;
    QString m_currentGerbLine;

    int m_lineNum = 0;
    int m_goId = 0;

    StepRepeatStr m_stepRepeat;
    QMap<QString, Component> components;
    QString refDes;
    QMap<int, Attr::Aperture> aperFunctionMap;

    Attr::File attFile;
    Attr::Aperture attAper;

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
    App app;
    AppSettings appSettings;
};
}
