#include "Constructive.h"
namespace TopoR {
void Constructive::BoardOutline::Shape::Shift(double x, double y) {
    //    if((std::dynamic_pointer_cast<IBaseFigure>(NonfilledFigure)) != nullptr)
    //        (std::dynamic_pointer_cast<IBaseFigure>(NonfilledFigure)).Shift(x, y);
}
void Constructive::BoardOutline::Shape::UnitsConvert(dist in_units, dist out_units) {
    //    lineWidth = Ut::UnitsConvert(lineWidth, in_units, out_units);
    //    if((std::dynamic_pointer_cast<IBaseFigure>(NonfilledFigure)) != nullptr)
    //        (std::dynamic_pointer_cast<IBaseFigure>(NonfilledFigure)).UnitsConvert(in_units, out_units);
}
void Constructive::BoardOutline::Voids::Shift(double x, double y) {
    //    if((std::dynamic_pointer_cast<IBaseFigure>(FilledFigure)) != nullptr)
    //        (std::dynamic_pointer_cast<IBaseFigure>(FilledFigure)).Shift(x, y);
}
void Constructive::BoardOutline::Voids::UnitsConvert(dist in_units, dist out_units) {
    //    lineWidth = Ut::UnitsConvert(lineWidth, in_units, out_units);
    //    if((std::dynamic_pointer_cast<IBaseFigure>(FilledFigure)) != nullptr)
    //        (std::dynamic_pointer_cast<IBaseFigure>(FilledFigure)).UnitsConvert(in_units, out_units);
}
bool Constructive::BoardOutline::ShouldSerialize_Contours() { return Contour.size(); }
bool Constructive::BoardOutline::ShouldSerialize_Voids() { return Voids.size(); }

void Constructive::MntholeInstance::Shift(double x, double y) {
    //    Org.Shift(x, y);
}
void Constructive::MntholeInstance::UnitsConvert(dist in_units, dist out_units) {
    //    Org.UnitsConvert(in_units, out_units);
}
bool Constructive::Keepout::Role::Trace::ShouldSerialize_LayersRefs() { return LayersRefs.size(); }
void Constructive::Keepout::Shift(double x, double y) {
    //    if((std::dynamic_pointer_cast<IBaseFigure>(FigureContPolyline)) != nullptr)
    //        (std::dynamic_pointer_cast<IBaseFigure>(FigureContPolyline)).Shift(x, y);
}
void Constructive::Keepout::UnitsConvert(dist in_units, dist out_units) {
    //    if((std::dynamic_pointer_cast<IBaseFigure>(FigureContPolyline)) != nullptr)
    //        (std::dynamic_pointer_cast<IBaseFigure>(FigureContPolyline)).UnitsConvert(in_units, out_units);
}
bool Constructive::ShouldSerialize_Mntholes() { return Mntholes.size(); }
bool Constructive::ShouldSerialize_MechLayerObjects() { return MechLayerObjects.size(); }
bool Constructive::ShouldSerialize_Texts() { return Texts.size(); }
bool Constructive::ShouldSerialize_Keepouts() { return Keepouts.size(); }
void Constructive::Shift(double x, double y) {
    //    {
    //        if((BoardOutline == nullptr ? nullptr : BoardOutline->Contours.size()) > 0)
    //            for(int i{}; i < BoardOutline->Contours.size(); i++)
    //                BoardOutline->Contours[i].Shift(x, y);
    //        if((BoardOutline == nullptr ? nullptr : BoardOutline->Voids.size()) > 0)
    //            for(int i{}; i < BoardOutline->Voids.size(); i++)
    //                BoardOutline->Voids[i].Shift(x, y);
    //    }
    //        for(int i{}; i < Mntholes.size(); i++)
    //            Mntholes[i].Shift(x, y);
    //        for(int i{}; i < MechLayerObjects.size(); i++)
    //            MechLayerObjects[i].Shift(x, y);
    //        for(int i{}; i < Texts.size(); i++)
    //            Texts[i].Shift(x, y);
    //        for(int i{}; i < Keepouts.size(); i++)
    //            Keepouts[i].Shift(x, y);
}
void Constructive::UnitsConvert(dist in_units, dist out_units) {
    //    if((BoardOutline == nullptr ? nullptr : ((BoardOutline->Contours.empty() ? nullptr : BoardOutline->Contours.size()))) > 0)
    //        for(int i{}; i < BoardOutline->Contours.size(); i++)
    //            BoardOutline->Contours[i].UnitsConvert(in_units, out_units);
    //    if((BoardOutline == nullptr ? nullptr : ((BoardOutline->Voids.empty() ? nullptr : BoardOutline->Voids.size()))) > 0)
    //        for(int i{}; i < BoardOutline->Voids.size(); i++)
    //            BoardOutline->Voids[i].UnitsConvert(in_units, out_units);
    //        for(int i{}; i < Mntholes.size(); i++)
    //            Mntholes[i].UnitsConvert(in_units, out_units);
    //        for(int i{}; i < MechLayerObjects.size(); i++)
    //            MechLayerObjects[i].UnitsConvert(in_units, out_units);
    //        for(int i{}; i < Texts.size(); i++)
    //            Texts[i].UnitsConvert(in_units, out_units);
    //        for(int i{}; i < Keepouts.size(); i++)
    //            Keepouts[i].UnitsConvert(in_units, out_units);
}
void Constructive::Add(Constructive a, bool boardOutline, bool mntholeInstances, bool details, bool texts, bool keepouts) {
    //    /* int l;
    //     if (boardOutline)
    //     {
    //         if (a?.BoardOutline?.Contours?.Count > 0)
    //         {
    //             if (BoardOutline == null)
    //                 BoardOutline = new BoardOutline();
    //             if (BoardOutline.Contours == null)
    //                 BoardOutline.Contours = (BoardOutline.Shape[])a.BoardOutline.Contours.Clone();
    //             else
    //             {
    //                 l = BoardOutline.Contours.Count;
    //                 Array.Resize(ref BoardOutline.Contours, l + a.BoardOutline.Contours.Count);
    //                 a.BoardOutline.Contours.CopyTo(BoardOutline.Contours, l);
    //             }
    //         }
    //         if (a?.BoardOutline?.Voids?.Count > 0)
    //         {
    //             if (BoardOutline == null)
    //                 BoardOutline = new BoardOutline();
    //             if (BoardOutline.Voids == null)
    //                 BoardOutline.Voids = (BoardOutline.Shape_Voids[])a.BoardOutline.Voids.Clone();
    //             else
    //             {
    //                 l = BoardOutline.Voids.Count;
    //                 Array.Resize(ref BoardOutline.Voids, l + a.BoardOutline.Voids.Count);
    //                 a.BoardOutline.Voids.CopyTo(BoardOutline.Voids, l);
    //             }
    //         }
    //     }
    //     if (mntholeInstances)
    //     {
    //         if (a?.Mntholes?.Count > 0)
    //         {
    //             if (Mntholes == null)
    //                 Mntholes = (MntholeInstance[])a.Mntholes.Clone();
    //             else
    //             {
    //                 l = Mntholes.Count;
    //                 Array.Resize(ref Mntholes, l + a.Mntholes.Count);
    //                 a.Mntholes.CopyTo(Mntholes, l);
    //             }
    //         }
    //     }
    //     if (details)
    //     {
    //         if (a?.MechLayerObjects?.Count > 0)
    //         {
    //             if (MechLayerObjects == null)
    //                 MechLayerObjects = (Detail[])a.MechLayerObjects.Clone();
    //             else
    //             {
    //                 l = MechLayerObjects.Count;
    //                 Array.Resize(ref MechLayerObjects, l + a.MechLayerObjects.Count);
    //                 a.MechLayerObjects.CopyTo(MechLayerObjects, l);
    //             }
    //         }
    //     }
    //     if (texts)
    //     {
    //         if (a?.Texts?.Count > 0)
    //         {
    //             if (Texts == null)
    //                 Texts = (Text[])a.Texts.Clone();
    //             else
    //             {
    //                 l = Texts.Count;
    //                 Array.Resize(ref Texts, l + a.Texts.Count);
    //                 a.Texts.CopyTo(Texts, l);
    //             }
    //         }
    //     }
    //     if (keepouts)
    //     {
    //         if (a?.Keepouts?.Count > 0)
    //         {
    //             if (Keepouts == null)
    //                 Keepouts = (Keepout[])a.Keepouts.Clone();
    //             else
    //             {
    //                 l = Keepouts.Count;
    //                 Array.Resize(ref Keepouts, l + a.Keepouts.Count);
    //                 a.Keepouts.CopyTo(Keepouts, l);
    //             }
    //         }
    //     }*/
}
} // namespace TopoR
