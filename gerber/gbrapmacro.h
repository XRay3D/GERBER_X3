#pragma once

#include "gbraperture.h"

namespace Gerber {

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
class ApMacro final : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format);
    ApMacro(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double) const;
    void drawC(const State& state, bool fl);

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    QString m_macro;
    QList<QString> m_modifiers;
    QMap<QString, double> m_coefficients;

    Path drawCenterLine(const QList<double>& mod);
    Path drawCircle(const QList<double>& mod);
    Path drawOutlineCustomPolygon(const QList<double>& mod);
    Path drawOutlineRegularPolygon(const QList<double>& mod);
    Path drawVectorLine(const QList<double>& mod);
    void drawMoire(const QList<double>& mod);
    void drawThermal(const QList<double>& mod);

    ////////////

    Polygon_2 RectanglePath(const Point_2& wh /*double width, double height*/, Transformation& t, const Point_2& center = {});

    Polygon_2 drawCenterLine(const State& state, const QList<double>& mod);
    Polygon_2 drawCircle(const State& state, const QList<double>& mod);
    Polygon_2 drawOutlineCustomPolygon(const State& state, const QList<double>& mod);
    Polygon_2 drawOutlineRegularPolygon(const State& state, const QList<double>& mod);
    Polygon_2 drawVectorLine(const State& state, const QList<double>& mod);
    void drawMoire(const State& state, QVector<QPair<bool, Polygon_2>>& items, const QList<double>& mod);
    void drawThermal(const State& state, QVector<QPair<bool, Polygon_2>>& items, const QList<double>& mod);
};
}
