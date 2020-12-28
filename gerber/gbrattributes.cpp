// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrattributes.h"
#include "gbrattrfilefunction.h"

///////////////////////////////////////
/// \brief Gerber::Attr::FileFunction::~FileFunction
///
Gerber::Attr::FileFunction::~FileFunction()
{
    if (m_data)
        delete m_data;
}

Gerber::Attr::FileFunction::eFunction Gerber::Attr::FileFunction::toFunction(const QString& key) { return eFunction(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data())); }

void Gerber::Attr::FileFunction::parse(const QStringList& list)
{
    switch (m_function = toFunction(list.first())) {
    case eFunction::Legend:
        m_data = new struct Legend(m_function, list.mid(1));
        break;
    case eFunction::Component:
        m_data = new struct Component(m_function, list.mid(1));
        break;
    case eFunction::Heatsinkmask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::ArrayDrawing:
        m_data = new struct ArrayDrawing(m_function, list.mid(1));
        break;
    case eFunction::AssemblyDrawing:
        m_data = new struct AssemblyDrawing(m_function, list.mid(1));
        break;
    case eFunction::Carbonmask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::Copper:
        m_data = new struct Copper(m_function, list.mid(1));
        break;
    case eFunction::Depthrout:
        m_data = new struct Depthrout(m_function, list.mid(1));
        break;
    case eFunction::Drillmap:
        m_data = new struct Drillmap(m_function, list.mid(1));
        break;
    case eFunction::FabricationDrawing:
        m_data = new struct FabricationDrawing(m_function, list.mid(1));
        break;
    case eFunction::Glue:
        m_data = new struct Glue(m_function, list.mid(1));
        break;
    case eFunction::Goldmask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::NonPlated:
        m_data = new struct NonPlated(m_function, list.mid(1));
        break;
    case eFunction::Other:
        m_data = new struct Other(m_function, list.mid(1));
        break;
    case eFunction::OtherDrawing:
        m_data = new struct OtherDrawing(m_function, list.mid(1));
        break;
    case eFunction::Pads:
        m_data = new struct Pads(m_function, list.mid(1));
        break;
    case eFunction::Paste:
        m_data = new struct Paste(m_function, list.mid(1));
        break;
    case eFunction::Peelablemask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::Plated:
        m_data = new struct Plated(m_function, list.mid(1));
        break;
    case eFunction::Profile:
        m_data = new struct Profile(m_function, list.mid(1));
        break;
    case eFunction::Silvermask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::Soldermask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::Tinmask:
        m_data = new struct Mask(m_function, list.mid(1));
        break;
    case eFunction::Vcut:
        m_data = new struct Vcut(m_function, list.mid(1));
        break;
    case eFunction::Vcutmap:
        m_data = new struct Vcutmap(m_function, list.mid(1));
        break;
    case eFunction::Viafill:
        m_data = new struct Viafill(m_function, list.mid(1));
        break;
    default:;
        throw QString("Unknownwn FileFunction: %1").arg(list.first());
    }
    qDebug() << __FUNCTION__ << m_function;
}
