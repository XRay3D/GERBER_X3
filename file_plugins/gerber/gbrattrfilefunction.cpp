// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrattrfilefunction.h"

namespace Gerber::Attr {
/////////////////////////////////////////////////////
/// \brief AbsctractData::AbsctractData
/// \param function
///
AbstrFileFunc::AbstrFileFunc(File::Function function)
    : function(function)
{
}
/////////////////////////////////////////////////////
/// \brief Copper::Copper
/// \param function
/// \param list
///
Copper::Copper(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , layer(toLayer(list.value(0)))
    , side(toSide(list.value(1)))
    , type(toType(list.value(2)))
{
    //    qDebug()
    //        << "\n\t" << layer
    //        << "\n\t" << side
    //        << "\n\t" << type;
}
/////////////////////////////////////////////////////
/// \brief Plated::Plated
/// \param function
/// \param list
///
Plated::Plated(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , layerFrom(list.value(0).toInt())
    , layerTo(list.value(1).toInt())
    , type(toType(list.value(2)))
    , label(toLabel(list.value(3)))
{
    //    qDebug()
    //        << "\n\t" << layerFrom
    //        << "\n\t" << layerTo
    //        << "\n\t" << type
    //        << "\n\t" << label;
}
/////////////////////////////////////////////////////
/// \brief NonPlated::NonPlated
/// \param function
/// \param list
///
NonPlated::NonPlated(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , layerFrom(list.value(0).toInt())
    , layerTo(list.value(1).toInt())
    , type(toType(list.value(2)))
    , label(toLabel(list.value(3)))
{
    //    qDebug()
    //        << "\n\t" << layerFrom
    //        << "\n\t" << layerTo
    //        << "\n\t" << type
    //        << "\n\t" << label;
}
/////////////////////////////////////////////////////
/// \brief Legend::Legend
/// \param function
/// \param list
///
Legend::Legend(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , side(toSide(list.value(0)))
    , index(list.size() > 1 ? list.value(1).toInt() : -1)
{
    //    qDebug()
    //        << "\n\t" << side
    //        << "\n\t" << index;
}
/////////////////////////////////////////////////////
/// \brief Soldermask::Soldermask
/// \param function
/// \param list
///
Mask::Mask(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , side(toSide(list.value(0)))
    , index(list.size() > 1 ? list.value(1).toInt() : -1)
    , type(toType(function))
{
    //    qDebug()
    //        << "\n\t" << side
    //        << "\n\t" << index
    //        << "\n\t" << type;
}
/////////////////////////////////////////////////////
/// \brief Profile::Profile
/// \param function
/// \param list
///
Profile::Profile(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , plated(toEdgePlated(list.value(0)))
{
    //    qDebug()
    //        << "\n\t" << plated;
}
/////////////////////////////////////////////////////
/// \brief Paste::Paste
/// \param function
/// \param list
///
Paste::Paste(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , side(toSide(list.value(0)))
{
    //    qDebug()
    //        << "\n\t" << side;
}
/////////////////////////////////////////////////////
/// \brief AssemblyDrawing::AssemblyDrawing
/// \param function
/// \param list
///
AssemblyDrawing::AssemblyDrawing(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , side(toSide(list.value(0)))
{
    //    qDebug()
    //        << "\n\t" << side;
}
/////////////////////////////////////////////////////
/// \brief Component::Component
/// \param function
/// \param list
///
Component::Component(File::Function function, const QStringList& list)
    : AbstrFileFunc(function)
    , layer(toLayer(list.value(0)))
    , side(toSide(list.value(1)))
{
    //    qDebug()
    //        << "\n\t" << layer
    //        << "\n\t" << side;
}

}
