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
    void draw(const State& state, bool fl);

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
};

}
