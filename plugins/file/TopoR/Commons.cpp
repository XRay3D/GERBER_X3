#include "Commons.h"

// namespace TopoR_PCB_Classes {

void base_coordinat::Shift(float x, float y) {
    _x += x;
    _y += y;
}

void base_coordinat::UnitsConvert(dist_ in_units, dist_ out_units) {
    _x = Ut::UnitsConvert(_x, in_units, out_units);
    _y = Ut::UnitsConvert(_y, in_units, out_units);
}

void SegmentLine::Shift(float x, float y) {
    if(_End != nullptr)
        _End->Shift(x, y);
}

void SegmentLine::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void SegmentArcCCW::Shift(float x, float y) {
    if(_Center != nullptr)
        _Center->Shift(x, y);
    if(_End != nullptr)
        _End->Shift(x, y);
}

void SegmentArcCCW::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Center != nullptr)
        _Center->UnitsConvert(in_units, out_units);
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void SegmentArcByMiddle::Shift(float x, float y) {
    if(_Middle != nullptr)
        _Middle->Shift(x, y);
    if(_End != nullptr)
        _End->Shift(x, y);
}

void SegmentArcByMiddle::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Middle != nullptr)
        _Middle->UnitsConvert(in_units, out_units);
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void ArcCCW::Shift(float x, float y) {
    if(_Start != nullptr)
        _Start->Shift(x, y);
    if(_Center != nullptr)
        _Center->Shift(x, y);
    if(_End != nullptr)
        _End->Shift(x, y);
}

void ArcCCW::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Start != nullptr)
        _Start->UnitsConvert(in_units, out_units);
    if(_Center != nullptr)
        _Center->UnitsConvert(in_units, out_units);
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void ArcByAngle::Shift(float x, float y) {
    if(_Start != nullptr)
        _Start->Shift(x, y);
    if(_End != nullptr)
        _End->Shift(x, y);
}

void ArcByAngle::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Start != nullptr)
        _Start->UnitsConvert(in_units, out_units);
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void ArcByMiddle::Shift(float x, float y) {
    if(_Start != nullptr)
        _Start->Shift(x, y);
    if(_Middle != nullptr)
        _Middle->Shift(x, y);
    if(_End != nullptr)
        _End->Shift(x, y);
}

void ArcByMiddle::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Start != nullptr)
        _Start->UnitsConvert(in_units, out_units);
    if(_Middle != nullptr)
        _Middle->UnitsConvert(in_units, out_units);
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void Circle::Shift(float x, float y) {
    if(_Center != nullptr)
        _Center->Shift(x, y);
}

void Circle::UnitsConvert(dist_ in_units, dist_ out_units) {
    _diameter = Ut::UnitsConvert(_diameter, in_units, out_units);
    if(_Center != nullptr)
        _Center->UnitsConvert(in_units, out_units);
}

bool Line::ShouldSerialize_Dots() {
    return _Dots.empty() ? false : _Dots.size() > 0;
}

void Line::Shift(float x, float y) {
    if(_Dots.empty() ? false : _Dots.size() > 0)
        for(int i = 0; i < _Dots.size(); i++)
            _Dots[i]->Shift(x, y);
}

void Line::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Dots.empty() ? false : _Dots.size() > 0)
        for(int i = 0; i < _Dots.size(); i++)
            _Dots[i]->UnitsConvert(in_units, out_units);
}

bool Polyline::ShouldSerialize_Segments() {
    return _Segments.empty() ? false : _Segments.size() > 0;
}

void Polyline::Shift(float x, float y) {
    if(_Start != nullptr)
        _Start->Shift(x, y);

    if(_Segments.empty() ? false : _Segments.size() > 0)
        for(int i = 0; i < _Segments.size(); i++)
            std::visit([&](auto&& s) { s.Shift(x, y); }, _Segments[i]);
}

void Polyline::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Start != nullptr)
        _Start->UnitsConvert(in_units, out_units);

    if(_Segments.empty() ? false : _Segments.size() > 0)
        for(int i = 0; i < _Segments.size(); i++)
            std::visit([&](auto&& s) { s.UnitsConvert(in_units, out_units); }, _Segments[i]);
}

void TrackArcCW::Shift(float x, float y) {
    if(_Center != nullptr)
        _Center->Shift(x, y);
    if(_End != nullptr)
        _End->Shift(x, y);
}

void TrackArcCW::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Center != nullptr)
        _Center->UnitsConvert(in_units, out_units);
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void TrackLine::Shift(float x, float y) {
    if(_End != nullptr)
        _End->Shift(x, y);
}

void TrackLine::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_End != nullptr)
        _End->UnitsConvert(in_units, out_units);
}

void Thermal::UnitsConvert(dist_ in_units, dist_ out_units) {
    _spokeWidth = Ut::UnitsConvert(_spokeWidth, in_units, out_units);
}

void Detail::Shift(float x, float y) {
    if(_Figure.index() != std::variant_npos)
        std::visit([&](auto&& s) { s.Shift(x, y); }, _Figure);
}

void Detail::UnitsConvert(dist_ in_units, dist_ out_units) {
    _lineWidth = Ut::UnitsConvert(_lineWidth, in_units, out_units);
    if(_Figure.index() != std::variant_npos)
        std::visit([&](auto&& s) { s.UnitsConvert(in_units, out_units); }, _Figure);
}

bool Text::getMirrorSpecified() const {
    return _mirror != Bool::off;
}

void Text::Shift(float x, float y) {
    if(_Org != nullptr)
        _Org->Shift(x, y);
}

void Text::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Org != nullptr)
        _Org->UnitsConvert(in_units, out_units);
}

float Ut::UnitsConvert(float value, dist_ in_units, dist_ out_units) {
    double k;
    switch(in_units) {
    case dist_::mkm:
        k = 0.001;
        break;
    case dist_::cm:
        k = 10;
        break;
    case dist_::dm:
        k = 100;
        break;
    case dist_::m:
        k = 1000;
        break;
    case dist_::mil:
        k = 0.0254000000000000002032;
        break;
    case dist_::inch:
        k = 25.4000000000000002032;
        break;
    case dist_::mm:
    default:
        k = 1;
        break;
    }
    switch(out_units) {
    case dist_::mkm:
        return static_cast<float>(value * k * 1000);
    case dist_::cm:
        return static_cast<float>(value * k * 0.1);
    case dist_::dm:
        return static_cast<float>(value * k * 0.01);
    case dist_::m:
        return static_cast<float>(value * k * 0.001);
    case dist_::mil:
        return static_cast<float>(value * k * 39.37007874015748);
    case dist_::inch:
        return static_cast<float>(value * k * 0.03937007874015748);
    case dist_::mm:
        return static_cast<float>(value * k);
    default:
        return value;
    }
}
// } // namespace TopoR_PCB_Classes
