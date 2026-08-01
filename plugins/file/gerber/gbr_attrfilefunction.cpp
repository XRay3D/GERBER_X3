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
#include "gbr_attrfilefunction.h"

namespace Gerber::Attr {
/////////////////////////////////////////////////////
/// \brief AbsctractData::AbsctractData
/// \param function
///
AbstrFileFunc::AbstrFileFunc(File::Function function)
    : function{function} {
}
/////////////////////////////////////////////////////
/// \brief Copper::Copper
/// \param function
/// \param list
///
Copper::Copper(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , layer(toLayer(list.value(0)))
    , side(toSide(list.value(1)))
    , type(toType(list.value(2))) {
    // qDebug()
    // << u"\n\t"_s << layer
    // << u"\n\t"_s << side
    // << u"\n\t"_s << type;
}
/////////////////////////////////////////////////////
/// \brief Plated::Plated
/// \param function
/// \param list
///
Plated::Plated(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , layerFrom(list.value(0).toInt())
    , layerTo(list.value(1).toInt())
    , type(toType(list.value(2)))
    , label(toLabel(list.value(3))) {
    // qDebug()
    // << u"\n\t"_s << layerFrom
    // << u"\n\t"_s << layerTo
    // << u"\n\t"_s << type
    // << u"\n\t"_s << label;
}
/////////////////////////////////////////////////////
/// \brief NonPlated::NonPlated
/// \param function
/// \param list
///
NonPlated::NonPlated(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , layerFrom(list.value(0).toInt())
    , layerTo(list.value(1).toInt())
    , type(toType(list.value(2)))
    , label(toLabel(list.value(3))) {
    // qDebug()
    // << u"\n\t"_s << layerFrom
    // << u"\n\t"_s << layerTo
    // << u"\n\t"_s << type
    // << u"\n\t"_s << label;
}
/////////////////////////////////////////////////////
/// \brief Legend::Legend
/// \param function
/// \param list
///
Legend::Legend(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , side(toSide(list.value(0)))
    , index(list.size() > 1 ? list.value(1).toInt() : -1) {
    // qDebug()
    // << u"\n\t"_s << side
    // << u"\n\t"_s << index;
}
/////////////////////////////////////////////////////
/// \brief Soldermask::Soldermask
/// \param function
/// \param list
///
Mask::Mask(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , side(toSide(list.value(0)))
    , index(list.size() > 1 ? list.value(1).toInt() : -1)
    , type(toType(function)) {
    // qDebug()
    // << u"\n\t"_s << side
    // << u"\n\t"_s << index
    // << u"\n\t"_s << type;
}
/////////////////////////////////////////////////////
/// \brief Profile::Profile
/// \param function
/// \param list
///
Profile::Profile(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , plated(toEdgePlated(list.value(0))) {
    // qDebug()
    // << u"\n\t"_s << plated;
}
/////////////////////////////////////////////////////
/// \brief Paste::Paste
/// \param function
/// \param list
///
Paste::Paste(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , side(toSide(list.value(0))) {
    // qDebug()
    // << u"\n\t"_s << side;
}
/////////////////////////////////////////////////////
/// \brief AssemblyDrawing::AssemblyDrawing
/// \param function
/// \param list
///
AssemblyDrawing::AssemblyDrawing(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , side(toSide(list.value(0))) {
    // qDebug()
    // << u"\n\t"_s << side;
}
/////////////////////////////////////////////////////
/// \brief Component::Component
/// \param function
/// \param list
///
Component::Component(File::Function function, const QStringList& list)
    : AbstrFileFunc{function}
    , layer(toLayer(list.value(0)))
    , side(toSide(list.value(1))) {
    // qDebug()
    // << u"\n\t"_s << layer
    // << u"\n\t"_s << side;
}

} // namespace Gerber::Attr

#include "moc_gbr_attrfilefunction.cpp"
