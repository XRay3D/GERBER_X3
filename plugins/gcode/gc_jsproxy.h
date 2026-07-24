/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "curve.h"
#include <QJSValue>
#include <QObject>

class QJSEngine;

namespace GCode {

class File;

class GcFileProxy : public QObject {
    Q_OBJECT

    // Tool and feed properties
    Q_PROPERTY(bool laser READ laser CONSTANT)
    Q_PROPERTY(double toolDiameter READ toolDiameter CONSTANT)
    Q_PROPERTY(double toolLength READ toolLength CONSTANT)
    Q_PROPERTY(double toolOneTurnCut READ toolOneTurnCut CONSTANT)
    Q_PROPERTY(double feedRate READ feedRate CONSTANT)
    Q_PROPERTY(double plungeRate READ plungeRate CONSTANT)
    Q_PROPERTY(int spindleSpeed READ spindleSpeed CONSTANT)
    Q_PROPERTY(QString strFeed READ strFeed CONSTANT)
    Q_PROPERTY(QString strPlungeFeed READ strPlungeFeed CONSTANT)
    Q_PROPERTY(QString strSpindle READ strSpindle CONSTANT)

    // Threading (Tool::ThreadMill) properties
    Q_PROPERTY(bool inside READ inside CONSTANT)     // true: internal thread (hole), false: external thread (rod)
    Q_PROPERTY(bool climb READ climb CONSTANT)       // true: climb milling, false: conventional milling
    Q_PROPERTY(bool leftHand READ leftHand CONSTANT) // true: left-hand thread
    Q_PROPERTY(double threadPitch READ threadPitch CONSTANT)       // Tool::passDepth() repurposed as thread pitch P
    Q_PROPERTY(double threadHoleDiam READ threadHoleDiam CONSTANT) // Tool::holeDiam(), pre-machined hole/rod diameter m
    Q_PROPERTY(bool circle READ circle CONSTANT)   // add a full circle pass at thread bottom
    Q_PROPERTY(bool chamfer READ chamfer CONSTANT) // cut a chamfer with the thread mill after threading
    Q_PROPERTY(int starts READ starts CONSTANT)    // number of thread starts N (multi-start thread)

    // Project settings
    Q_PROPERTY(double safeZ READ safeZ CONSTANT)
    Q_PROPERTY(double plunge READ plunge CONSTANT)
    Q_PROPERTY(double clearance READ clearance CONSTANT)
    Q_PROPERTY(int stepsX READ stepsX CONSTANT)
    Q_PROPERTY(int stepsY READ stepsY CONSTANT)
    Q_PROPERTY(double workWidth READ workWidth CONSTANT)
    Q_PROPERTY(double workHeight READ workHeight CONSTANT)
    Q_PROPERTY(double spaceX READ spaceX CONSTANT)
    Q_PROPERTY(double spaceY READ spaceY CONSTANT)

    // Current depth (updated by savePath spiral logic)
    Q_PROPERTY(double z READ zVal WRITE setZ)

public:
    explicit GcFileProxy(File* file, QJSEngine* engine, QObject* parent = nullptr);

    bool laser() const;
    double toolDiameter() const;
    double toolLength() const;
    double toolOneTurnCut() const;
    double feedRate() const;
    double plungeRate() const;
    int spindleSpeed() const;
    QString strFeed() const;
    QString strPlungeFeed() const;
    QString strSpindle() const;
    bool inside() const;
    bool climb() const;
    bool leftHand() const;
    double threadPitch() const;
    double threadHoleDiam() const;
    bool circle() const;
    bool chamfer() const;
    int starts() const;
    double safeZ() const;
    double plunge() const;
    double clearance() const;
    int stepsX() const;
    int stepsY() const;
    double workWidth() const;
    double workHeight() const;
    double spaceX() const;
    double spaceY() const;
    double zVal() const;
    void setZ(double z);

    // Return all tool paths transformed for tile offset (ox, oy).
    // Result: array[pathssIdx][pathIdx][vertexIdx] = {x, y, type, cx, cy}
    // Each inner array also has .closed (bool) and .perimetr (double) properties.
    Q_INVOKABLE QJSValue getToolPaths(double ox, double oy);

    // Return array of cut depths (negative values, deepest last).
    Q_INVOKABLE QJSValue getDepths();

    // Emit G0 rapid to (x, y), plunge, and touch Z0 surface (updates file z_).
    Q_INVOKABLE void startPath(double x, double y);

    // Lift tool to clearance height.
    Q_INVOKABLE void endPath();

    // Append a raw gcode line to the output.
    Q_INVOKABLE void addLine(const QString& line);

    // Generate gcode lines for one curve from the last getToolPaths() result.
    // pathssIdx / pathIdx select the Curvess[i][j] curve.
    // reversed = true uses the curve in reverse direction (zigzag alternate pass).
    // perimetr > 0 && depth != 0 enables spiral ramp; otherwise flat cut.
    // Returns JS array of strings.
    Q_INVOKABLE QJSValue savePathLines(int pathssIdx, int pathIdx, bool reversed, double perimetr, double depth);

    // Format a list of gcode tokens, suppressing unchanged values per format flags.
    // parts: JS array of strings such as [g0(), fmtX(1.0), fmtY(2.0)].
    Q_INVOKABLE QString formatted(const QJSValue& parts);

    Q_INVOKABLE QString g0();
    Q_INVOKABLE QString g1();
    Q_INVOKABLE QString g2();
    Q_INVOKABLE QString g3();
    Q_INVOKABLE QString fmtX(double v);
    Q_INVOKABLE QString fmtY(double v);
    Q_INVOKABLE QString fmtZ(double v);
    Q_INVOKABLE QString fmtI(double v);
    Q_INVOKABLE QString fmtJ(double v);
    Q_INVOKABLE QString fmtS(int v);
    Q_INVOKABLE QString fmtF(double v);

private:
    File* file_;
    QJSEngine* engine_;
    Curvess cachedPathss_;
};

} // namespace GCode
