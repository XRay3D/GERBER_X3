// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
﻿#include "Constructive.h"

    // namespace TopoR_PCB_Classes {

    void
    Constructive::BoardOutline::Shape_Contour::Shift(float x, float y) {
    if(_NonfilledFigure.index() != std::variant_npos)
        std::visit([&](auto&& f) { f.Shift(x, y); }, _NonfilledFigure);
}

void Constructive::BoardOutline::Shape_Contour::UnitsConvert(dist_ in_units, dist_ out_units) {
    _lineWidth = Ut::UnitsConvert(_lineWidth, in_units, out_units);
    if(_NonfilledFigure.index() != std::variant_npos)
        std::visit([&](auto&& f) { f.UnitsConvert(in_units, out_units); }, _NonfilledFigure);
}

void Constructive::BoardOutline::Shape_Voids::Shift(float x, float y) {
    if(_FilledFigure.index() != std::variant_npos)
        std::visit([&](auto&& f) { f.Shift(x, y); }, _FilledFigure);
}

void Constructive::BoardOutline::Shape_Voids::UnitsConvert(dist_ in_units, dist_ out_units) {
    _lineWidth = Ut::UnitsConvert(_lineWidth, in_units, out_units);
    if(_FilledFigure.index() != std::variant_npos)
        std::visit([&](auto&& f) { f.UnitsConvert(in_units, out_units); }, _FilledFigure);
}

bool Constructive::BoardOutline::ShouldSerialize_Contours() {
    return _Contours.empty() ? false : _Contours.size() > 0;
}

bool Constructive::BoardOutline::ShouldSerialize_Voids() {
    return _Voids.empty() ? false : _Voids.size() > 0;
}

bool Constructive::MntholeInstance::getFixedSpecified() const {
    return _fixed != Bool::off;
}

void Constructive::MntholeInstance::Shift(float x, float y) {
    if(_Org != nullptr)
        _Org->Shift(x, y);
}

void Constructive::MntholeInstance::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_Org != nullptr)
        _Org->UnitsConvert(in_units, out_units);
}

bool Constructive::Keepout_Сonstructive::Role::Trace::ShouldSerialize_LayersRefs() {
    return _LayersRefs.empty() ? false : _LayersRefs.size() > 0;
}

void Constructive::Keepout_Сonstructive::Shift(float x, float y) {
    if(_FigureContPolyline.index() != std::variant_npos)
        std::visit([&](auto&& f) { f.Shift(x, y); }, _FigureContPolyline);
}

void Constructive::Keepout_Сonstructive::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_FigureContPolyline.index() != std::variant_npos)
        std::visit([&](auto&& f) { f.UnitsConvert(in_units, out_units); }, _FigureContPolyline);
}

bool Constructive::ShouldSerialize_Mntholes() {
    return _Mntholes.empty() ? false : _Mntholes.size() > 0;
}

bool Constructive::ShouldSerialize_MechLayerObjects() {
    return _MechLayerObjects.empty() ? false : _MechLayerObjects.size() > 0;
}

bool Constructive::ShouldSerialize_Texts() {
    return _Texts.empty() ? false : _Texts.size() > 0;
}

bool Constructive::ShouldSerialize_Keepouts() {
    return _Keepouts.empty() ? false : _Keepouts.size() > 0;
}

void Constructive::Shift(float x, float y) {
    if(_BoardOutline != nullptr) {
        if(_BoardOutline == nullptr ? false : _BoardOutline->_Contours.size() > 0)
            for(int i = 0; i < _BoardOutline->_Contours.size(); i++)
                _BoardOutline->_Contours[i]->Shift(x, y);
        if(_BoardOutline == nullptr ? false : _BoardOutline->_Voids.size() > 0)
            for(int i = 0; i < _BoardOutline->_Voids.size(); i++)
                _BoardOutline->_Voids[i]->Shift(x, y);
    }

    if(_Mntholes.empty() ? false : _Mntholes.size() > 0)
        for(int i = 0; i < _Mntholes.size(); i++)
            _Mntholes[i]->Shift(x, y);

    if(_MechLayerObjects.empty() ? false : _MechLayerObjects.size() > 0)
        for(int i = 0; i < _MechLayerObjects.size(); i++)
            _MechLayerObjects[i]->Shift(x, y);

    if(_Texts.empty() ? false : _Texts.size() > 0)
        for(int i = 0; i < _Texts.size(); i++)
            _Texts[i]->Shift(x, y);

    if(_Keepouts.empty() ? false : _Keepouts.size() > 0)
        for(int i = 0; i < _Keepouts.size(); i++)
            _Keepouts[i]->Shift(x, y);
}

void Constructive::UnitsConvert(dist_ in_units, dist_ out_units) {
    if(_BoardOutline == nullptr ? false : (_BoardOutline->_Contours.empty() ? false : _BoardOutline->_Contours.size()) > 0)
        for(int i = 0; i < _BoardOutline->_Contours.size(); i++)
            _BoardOutline->_Contours[i]->UnitsConvert(in_units, out_units);
    if(_BoardOutline == nullptr ? false : (_BoardOutline->_Voids.empty() ? false : _BoardOutline->_Voids.size()) > 0)
        for(int i = 0; i < _BoardOutline->_Voids.size(); i++)
            _BoardOutline->_Voids[i]->UnitsConvert(in_units, out_units);

    if(_Mntholes.empty() ? false : _Mntholes.size() > 0)
        for(int i = 0; i < _Mntholes.size(); i++)
            _Mntholes[i]->UnitsConvert(in_units, out_units);

    if(_MechLayerObjects.empty() ? false : _MechLayerObjects.size() > 0)
        for(int i = 0; i < _MechLayerObjects.size(); i++)
            _MechLayerObjects[i]->UnitsConvert(in_units, out_units);

    if(_Texts.empty() ? false : _Texts.size() > 0)
        for(int i = 0; i < _Texts.size(); i++)
            _Texts[i]->UnitsConvert(in_units, out_units);

    if(_Keepouts.empty() ? false : _Keepouts.size() > 0)
        for(int i = 0; i < _Keepouts.size(); i++)
            _Keepouts[i]->UnitsConvert(in_units, out_units);
}

void Constructive::Add(Constructive* a, bool boardOutline, bool mntholeInstances, bool details, bool texts, bool keepouts) {
    /* int l;
     if (boardOutline)
     {
         if (a?._BoardOutline?._Contours?.Count > 0)
         {
             if (_BoardOutline == null)
                 _BoardOutline = new BoardOutline();

             if (_BoardOutline._Contours == null)
                 _BoardOutline._Contours = (BoardOutline.Shape_Contour[])a._BoardOutline._Contours.Clone();
             else
             {
                 l = _BoardOutline._Contours.Count;
                 Array.Resize(ref _BoardOutline._Contours, l + a._BoardOutline._Contours.Count);
                 a._BoardOutline._Contours.CopyTo(_BoardOutline._Contours, l);
             }
         }

         if (a?._BoardOutline?._Voids?.Count > 0)
         {
             if (_BoardOutline == null)
                 _BoardOutline = new BoardOutline();

             if (_BoardOutline._Voids == null)
                 _BoardOutline._Voids = (BoardOutline.Shape_Voids[])a._BoardOutline._Voids.Clone();
             else
             {
                 l = _BoardOutline._Voids.Count;
                 Array.Resize(ref _BoardOutline._Voids, l + a._BoardOutline._Voids.Count);
                 a._BoardOutline._Voids.CopyTo(_BoardOutline._Voids, l);
             }
         }
     }

     if (mntholeInstances)
     {
         if (a?._Mntholes?.Count > 0)
         {
             if (_Mntholes == null)
                 _Mntholes = (MntholeInstance[])a._Mntholes.Clone();
             else
             {
                 l = _Mntholes.Count;
                 Array.Resize(ref _Mntholes, l + a._Mntholes.Count);
                 a._Mntholes.CopyTo(_Mntholes, l);
             }
         }
     }

     if (details)
     {
         if (a?._MechLayerObjects?.Count > 0)
         {
             if (_MechLayerObjects == null)
                 _MechLayerObjects = (Detail[])a._MechLayerObjects.Clone();
             else
             {
                 l = _MechLayerObjects.Count;
                 Array.Resize(ref _MechLayerObjects, l + a._MechLayerObjects.Count);
                 a._MechLayerObjects.CopyTo(_MechLayerObjects, l);
             }
         }
     }

     if (texts)
     {
         if (a?._Texts?.Count > 0)
         {
             if (_Texts == null)
                 _Texts = (Text[])a._Texts.Clone();
             else
             {
                 l = _Texts.Count;
                 Array.Resize(ref _Texts, l + a._Texts.Count);
                 a._Texts.CopyTo(_Texts, l);
             }
         }
     }

     if (keepouts)
     {
         if (a?._Keepouts?.Count > 0)
         {
             if (_Keepouts == null)
                 _Keepouts = (Keepout_Сonstructive[])a._Keepouts.Clone();
             else
             {
                 l = _Keepouts.Count;
                 Array.Resize(ref _Keepouts, l + a._Keepouts.Count);
                 a._Keepouts.CopyTo(_Keepouts, l);
             }
         }
     }*/
}
// } // namespace TopoR_PCB_Classes
