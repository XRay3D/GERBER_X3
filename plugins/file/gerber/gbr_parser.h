/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gbr_attributes.h"
#include "gbr_types.h"
#include "gbrcomp_onent.h"

class FilePlugin;

namespace Gerber {

class ApBlock;

class Parser {
    FilePlugin* const interface;

public:
    Parser(FilePlugin* interface);

protected:
    void parseLines(const QString& gerberLines, const QString& fileName);

    mvector<QString> cleanAndFormatFile(QString data);
    double arcAngle(double start, double stop);
    double toDouble(const QString& Str, bool scale = false, bool inchControl = true);
    bool parseNumber(QString Str, Point::Type& val, int integer, int decimal);

    void addPath();
    void addFlash();

    void reset(const QString& fileName);
    void resetStep();

    Point parsePosition(const QString& xyStr);
    Path arc(const Point& center, double radius, double start, double stop);
    Path arc(Point p1, Point p2, Point center);

    Paths createLine();
    Paths createPolygon();

    Clipper clipper_;
    ClipperOffset offset_;

    QMap<QString, QString> apertureMacro_;

    struct WorkingType {
        enum eWT {
            Normal,
            StepRepeat,
            ApertureBlock,
        };
        eWT workingType = Normal;
        int apertureBlockId = 0;
    };

    QStack<WorkingType> abSrIdStack_;

    Path path_;
    State state_;
    QString currentGerbLine_;

    int lineNum_ = 0;
    int goId_ = 0;

    StepRepeatStr stepRepeat_;
    QMap<QString, Comp::Component> components;
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
    bool parseLoadName(const QString& gLine);
    bool parseStepRepeat(const QString& gLine);
    bool parseTransformations(const QString& gLine);
    bool parseUnitMode(const QString& gLine);
    void closeStepRepeat();

    ApBlock* apBlock(int id);

    File* file = nullptr;
};

} // namespace Gerber
