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
namespace Gerber::Attr {

File::StdAttr File::toStdAttr(const QString& key)
{
    return StdAttr(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()));
}

File::ePart File::toPart(const QString& key)
{
    return ePart(staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().data()));
}

File::eFilePolarity File::toFilePolarityValue(const QString& key)
{
    return eFilePolarity(staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().data()));
}

File::eFunction File::toFunction(const QString& key)
{
    return eFunction(staticMetaObject.enumerator(3).keyToValue(key.toLocal8Bit().data()));
}

void File::parse(const QStringList& list)
{
    switch (toStdAttr(list.first())) {
    case StdAttr::Part:
        part = list.mid(1);
        break;
    case StdAttr::FileFunction:
        switch (const auto function = toFunction(list[1]); function) {
        case eFunction::Legend:
            fileFunction = new struct Legend(function, list.mid(2));
            break;
        case eFunction::Component:
            fileFunction = new struct Component(function, list.mid(2));
            break;
        case eFunction::Heatsinkmask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::ArrayDrawing:
            fileFunction = new struct ArrayDrawing(function, list.mid(2));
            break;
        case eFunction::AssemblyDrawing:
            fileFunction = new struct AssemblyDrawing(function, list.mid(2));
            break;
        case eFunction::Carbonmask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::Copper:
            fileFunction = new struct Copper(function, list.mid(2));
            break;
        case eFunction::Depthrout:
            fileFunction = new struct Depthrout(function, list.mid(2));
            break;
        case eFunction::Drillmap:
            fileFunction = new struct Drillmap(function, list.mid(2));
            break;
        case eFunction::FabricationDrawing:
            fileFunction = new struct FabricationDrawing(function, list.mid(2));
            break;
        case eFunction::Glue:
            fileFunction = new struct Glue(function, list.mid(2));
            break;
        case eFunction::Goldmask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::NonPlated:
            fileFunction = new struct NonPlated(function, list.mid(2));
            break;
        case eFunction::Other:
            fileFunction = new struct Other(function, list.mid(2));
            break;
        case eFunction::OtherDrawing:
            fileFunction = new struct OtherDrawing(function, list.mid(2));
            break;
        case eFunction::Pads:
            fileFunction = new struct Pads(function, list.mid(2));
            break;
        case eFunction::Paste:
            fileFunction = new struct Paste(function, list.mid(2));
            break;
        case eFunction::Peelablemask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::Plated:
            fileFunction = new struct Plated(function, list.mid(2));
            break;
        case eFunction::Profile:
            fileFunction = new struct Profile(function, list.mid(2));
            break;
        case eFunction::Silvermask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::Soldermask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::Tinmask:
            fileFunction = new struct Mask(function, list.mid(2));
            break;
        case eFunction::Vcut:
            fileFunction = new struct Vcut(function, list.mid(2));
            break;
        case eFunction::Vcutmap:
            fileFunction = new struct Vcutmap(function, list.mid(2));
            break;
        case eFunction::Viafill:
            fileFunction = new struct Viafill(function, list.mid(2));
            break;
        default:;
            throw QString("Unknownwn File: %1").arg(list.first());
        }
        break;
    case StdAttr::FilePolarity:
        filePolarity = toFilePolarityValue(list.last());
        break;
    case StdAttr::SameCoordinates:
        sameCoordinates = list.mid(1);
        break;
    case StdAttr::CreationDate:
        creationDate = list.last();
        break;
    case StdAttr::GenerationSoftware:
        generationSoftware = list.mid(1);
        break;
    case StdAttr::ProjectId:
        projectId = list.mid(1);
        break;
    case StdAttr::MD5:
        md5 = list.last();
        break;
    default:;
        custom[list.first()] = list.mid(1);
        //qDebug() << __FUNCTION__ << "custom" << custom;
    }
}

}
