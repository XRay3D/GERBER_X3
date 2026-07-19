/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include <QPainterPath>
#include <QTransform>
#include <QXmlStreamReader>

namespace Svg {

class File;

class Parser {
public:
    bool parseFile(const QString& fileName, File* file);

private:
    void parseElement(QXmlStreamReader& xml, const QTransform& parentTransform);

    static double parseLength(QStringView str, double viewportDim = 0.0);
    static QTransform parseTransform(QStringView str);
    static QPolygonF parsePoints(QStringView str);
    static QPainterPath parseSvgPathData(QStringView d);

    static void svgArcTo(QPainterPath& path, QPointF p0, double rx, double ry,
        double xRot, bool largeArc, bool sweep, QPointF p1);

    static QPointF nextCoord(QStringView str, int& pos);
    static double nextNumber(QStringView str, int& pos);

    File* file_{};
    QTransform toMm_; // transform from SVG user units to mm (includes Y-flip)
};

} // namespace Svg
