#include "Constructive.h"
namespace TopoR {
// void Constructive::BoardOutline::Shape_Contour::Shift(double x, double y) {
//     /*   if(_NonfilledFigure)
//            (std::dynamic_pointer_cast<IBaseFigure>(_NonfilledFigure)).value().Shift(x, y);*/
// }
// void Constructive::BoardOutline::Shape_Contour::UnitsConvert(dist in_units, dist out_units) {
//     /*   lineWidth_ = Ut::UnitsConvert(_lineWidth, in_units, out_units);
//        if(_NonfilledFigure)
//            (std::dynamic_pointer_cast<IBaseFigure>(_NonfilledFigure)).value().UnitsConvert(in_units, out_units);*/
// }
// void Constructive::BoardOutline::Shape_Voids::Shift(double x, double y) {
//     /*    if(_FilledFigure)
//             (std::dynamic_pointer_cast<IBaseFigure>(_FilledFigure)).value().Shift(x, y);*/
// }
// void Constructive::BoardOutline::Shape_Voids::UnitsConvert(dist in_units, dist out_units) {
//     /*   lineWidth_ = Ut::UnitsConvert(_lineWidth, in_units, out_units);
//        if(_FilledFigure)
//            (std::dynamic_pointer_cast<IBaseFigure>(_FilledFigure)).value().UnitsConvert(in_units, out_units);*/
// }
// void Constructive::MntholeInstance::Shift(double x, double y) {
//     /**  if(_Org)
//           Org.value().Shift(x, y);*/
// }
// void Constructive::MntholeInstance::UnitsConvert(dist in_units, dist out_units) {
//     /*  if(_Org)
//           Org.value().UnitsConvert(in_units, out_units);*/
// }
// void Constructive::Keepout_Сonstructive::Shift(double x, double y) {
//     /* if(_FigureContPolyline)
//          (std::dynamic_pointer_cast<IBaseFigure>(_FigureContPolyline)).value().Shift(x, y);*/
// }
// void Constructive::Keepout_Сonstructive::UnitsConvert(dist in_units, dist out_units) {
//     /*  if(_FigureContPolyline)
//           (std::dynamic_pointer_cast<IBaseFigure>(_FigureContPolyline)).value().UnitsConvert(in_units, out_units);*/
// }
// void Constructive::Shift(double x, double y) {
//     /*  if(_BoardOutline) {
//            if(_BoardOutline.size())
//                for(int i = 0; i < BoardOutline.value()._Contours.size(); i++)
//                    BoardOutline_->_Contours[i].value().Shift(x, y);
//            if(_BoardOutline.size())
//                for(int i = 0; i < BoardOutline.value()._Voids.size(); i++)
//                    BoardOutline_->_Voids[i].value().Shift(x, y);
//        }
//        if(_Mntholes.size())
//            for(int i = 0; i < Mntholes.size(); i++)
//                Mntholes_[i].value().Shift(x, y);
//        if(_MechLayerObjects.size())
//            for(int i = 0; i < MechLayerObjects.size(); i++)
//                MechLayerObjects_[i].value().Shift(x, y);
//        if(_Texts.size())
//            for(int i = 0; i < Texts.size(); i++)
//                Texts_[i].value().Shift(x, y);
//        if(_Keepouts.size())
//            for(int i = 0; i < Keepouts.size(); i++)
//                Keepouts_[i].value().Shift(x, y);*/
// }
// void Constructive::UnitsConvert(dist in_units, dist out_units) {
//     /*   if(_BoardOutline.size())
//            for(int i = 0; i < BoardOutline.value()._Contours.size(); i++)
//                BoardOutline_->_Contours[i].value().UnitsConvert(in_units, out_units);
//        if(_BoardOutline.size())
//            for(int i = 0; i < BoardOutline.value()._Voids.size(); i++)
//                BoardOutline_->_Voids[i].value().UnitsConvert(in_units, out_units);
//        if(_Mntholes.size())
//            for(int i = 0; i < Mntholes.size(); i++)
//                Mntholes_[i].value().UnitsConvert(in_units, out_units);
//        if(_MechLayerObjects.size())
//            for(int i = 0; i < MechLayerObjects.size(); i++)
//                MechLayerObjects_[i].value().UnitsConvert(in_units, out_units);
//        if(_Texts.size())
//            for(int i = 0; i < Texts.size(); i++)
//                Texts_[i].value().UnitsConvert(in_units, out_units);
//        if(_Keepouts.size())
//            for(int i = 0; i < Keepouts.size(); i++)
//                Keepouts_[i].value().UnitsConvert(in_units, out_units);*/
// }
// void Constructive::Add(Constructive a, bool boardOutline, bool mntholeInstances, bool details, bool texts, bool keepouts) {
//     /* int l;
//      if (boardOutline)
//      {
//          if a.size())
//          {
//              if (_BoardOutline == null)
//                  BoardOutline_ = new BoardOutline();
//              if (_BoardOutline._Contours == null)
//                  BoardOutline._Contours = (BoardOutline.Shape_Contour[])a._BoardOutline._Contours.Clone();
//              else
//              {
//                  l = BoardOutline._Contours.Count;
//                  Array.Resize(ref BoardOutline._Contours, l + a._BoardOutline._Contours.Count);
//                  a._BoardOutline._Contours.CopyTo(_BoardOutline._Contours, l);
//              }
//          }
//          if a.size())
//          {
//              if (_BoardOutline == null)
//                  BoardOutline_ = new BoardOutline();
//              if (_BoardOutline._Voids == null)
//                  BoardOutline._Voids = (BoardOutline.Shape_Voids[])a._BoardOutline._Voids.Clone();
//              else
//              {
//                  l = BoardOutline._Voids.Count;
//                  Array.Resize(ref BoardOutline._Voids, l + a._BoardOutline._Voids.Count);
//                  a._BoardOutline._Voids.CopyTo(_BoardOutline._Voids, l);
//              }
//          }
//      }
//      if (mntholeInstances)
//      {
//          if a.size())
//          {
//              if (_Mntholes == null)
//                  Mntholes_ = (MntholeInstance[])a._Mntholes.Clone();
//              else
//              {
//                  l = Mntholes.Count;
//                  Array.Resize(ref Mntholes_, l + a._Mntholes.Count);
//                  a._Mntholes.CopyTo(_Mntholes, l);
//              }
//          }
//      }
//      if (details)
//      {
//          if a.size())
//          {
//              if (_MechLayerObjects == null)
//                  MechLayerObjects_ = (Detail[])a._MechLayerObjects.Clone();
//              else
//              {
//                  l = MechLayerObjects.Count;
//                  Array.Resize(ref MechLayerObjects_, l + a._MechLayerObjects.Count);
//                  a._MechLayerObjects.CopyTo(_MechLayerObjects, l);
//              }
//          }
//      }
//      if (texts)
//      {
//          if a.size())
//          {
//              if (_Texts == null)
//                  Texts_ = (Text[])a._Texts.Clone();
//              else
//              {
//                  l = Texts.Count;
//                  Array.Resize(ref Texts_, l + a._Texts.Count);
//                  a._Texts.CopyTo(_Texts, l);
//              }
//          }
//      }
//      if (keepouts)
//      {
//          if a.size())
//          {
//              if (_Keepouts == null)
//                  Keepouts_ = (Keepout_Сonstructive[])a._Keepouts.Clone();
//              else
//              {
//                  l = Keepouts.Count;
//                  Array.Resize(ref Keepouts_, l + a._Keepouts.Count);
//                  a._Keepouts.CopyTo(_Keepouts, l);
//              }
//          }
//      }*/
// }

} // namespace TopoR
