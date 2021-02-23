// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*          *
* Author    :  Damir Bakiev *
* Version   :  na           *
* Date      :  01 February 2020              *
* Website   :  na           *
* Copyright :  Damir Bakiev 2016-2021        *
*          *
* License: *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt       *
*          *
*******************************************************************************/
#include "gbrattributes.h"
#include "gbrattraperfunction.h"
#include "gbrattrfilefunction.h"

namespace Gerber::Attr {

File::StdAttr File::toStdAttr(const QString& key)
{
    return static_cast<StdAttr>(
        staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()));
}

File::ePart File::toPart(const QString& key)
{
    return static_cast<ePart>(
        staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().data()));
}

File::eFilePolarity File::toFilePolarityValue(const QString& key)
{
    return static_cast<eFilePolarity>(
        staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().data()));
}

File::Function File::toFunction(const QString& key)
{
    return static_cast<Function>(
        staticMetaObject.enumerator(3).keyToValue(key.toLocal8Bit().data()));
}

void File::parse(const QStringList& list)
{
    switch (toStdAttr(list.first())) {
    case StdAttr::Part:
        part = list.mid(1);
        break;
    case StdAttr::FileFunction:
        switch (const auto function = toFunction(list[1]); function) {
        case Function::Legend:
            m_function = std::make_shared<struct Legend>(function, list.mid(2));
            break;
        case Function::Component:
            m_function = std::make_shared<struct Component>(function, list.mid(2));
            break;
        case Function::Heatsinkmask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::ArrayDrawing:
            m_function = std::make_shared<struct ArrayDrawing>(function, list.mid(2));
            break;
        case Function::AssemblyDrawing:
            m_function = std::make_shared<struct AssemblyDrawing>(function, list.mid(2));
            break;
        case Function::Carbonmask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Copper:
            m_function = std::make_shared<struct Copper>(function, list.mid(2));
            break;
        case Function::Depthrout:
            m_function = std::make_shared<struct Depthrout>(function, list.mid(2));
            break;
        case Function::Drillmap:
            m_function = std::make_shared<struct Drillmap>(function, list.mid(2));
            break;
        case Function::FabricationDrawing:
            m_function = std::make_shared<struct FabricationDrawing>(function, list.mid(2));
            break;
        case Function::Glue:
            m_function = std::make_shared<struct Glue>(function, list.mid(2));
            break;
        case Function::Goldmask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::NonPlated:
            m_function = std::make_shared<struct NonPlated>(function, list.mid(2));
            break;
        case Function::Other:
            m_function = std::make_shared<struct Other>(function, list.mid(2));
            break;
        case Function::OtherDrawing:
            m_function = std::make_shared<struct OtherDrawing>(function, list.mid(2));
            break;
        case Function::Pads:
            m_function = std::make_shared<struct Pads>(function, list.mid(2));
            break;
        case Function::Paste:
            m_function = std::make_shared<struct Paste>(function, list.mid(2));
            break;
        case Function::Peelablemask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Plated:
            m_function = std::make_shared<struct Plated>(function, list.mid(2));
            break;
        case Function::Profile:
            m_function = std::make_shared<struct Profile>(function, list.mid(2));
            break;
        case Function::Silvermask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Soldermask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Tinmask:
            m_function = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Vcut:
            m_function = std::make_shared<struct Vcut>(function, list.mid(2));
            break;
        case Function::Vcutmap:
            m_function = std::make_shared<struct Vcutmap>(function, list.mid(2));
            break;
        case Function::Viafill:
            m_function = std::make_shared<struct Viafill>(function, list.mid(2));
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
        //qDebug() << "custom" << custom;
    }
}
///////////////////////////////////////////////////////////////////////////////////
/// \brief Aperture::value
/// \param key
/// \return
///
int Aperture::value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }

Aperture::StdAttr Aperture::toStdAttr(const QString& key)
{
    return static_cast<StdAttr>(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()));
}

Aperture::Function Aperture::toFunction(const QString& key)
{
    return static_cast<Function>(staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().data()));
}

void Aperture::parse(const QStringList& list)
{
    switch (toStdAttr(list.first())) {
    case StdAttr::AperFunction:
        //qDebug() << list;
        if (m_function)
            m_function.reset();
        switch (const auto function = toFunction(list[1]); function) {
        case ViaDrill:
            m_function = std::make_shared<struct ViaDrill>(function, list.mid(2));
            break;
        case BackDrill:
            m_function = std::make_shared<struct BackDrill>(function, list.mid(2));
            break;
        case ComponentDrill:
            m_function = std::make_shared<struct ComponentDrill>(function, list.mid(2));
            break;
        case MechanicalDrill:
            m_function = std::make_shared<struct MechanicalDrill>(function, list.mid(2));
            break;
        case CastellatedDrill:
            m_function = std::make_shared<struct CastellatedDrill>(function, list.mid(2));
            break;
        case OtherDrill:
            m_function = std::make_shared<struct OtherDrill>(function, list.mid(2));
            break;
        case ComponentPad:
            m_function = std::make_shared<struct ComponentPad>(function, list.mid(2));
            break;
        case SMDPad:
            m_function = std::make_shared<struct SMDPad>(function, list.mid(2));
            break;
        case BGAPad:
            m_function = std::make_shared<struct BGAPad>(function, list.mid(2));
            break;
        case ConnectorPad:
            m_function = std::make_shared<struct ConnectorPad>(function, list.mid(2));
            break;
        case HeatsinkPad:
            m_function = std::make_shared<struct HeatsinkPad>(function, list.mid(2));
            break;
        case ViaPad:
            m_function = std::make_shared<struct ViaPad>(function, list.mid(2));
            break;
        case TestPad:
            m_function = std::make_shared<struct TestPad>(function, list.mid(2));
            break;
        case CastellatedPad:
            m_function = std::make_shared<struct CastellatedPad>(function, list.mid(2));
            break;
        case FiducialPad:
            m_function = std::make_shared<struct FiducialPad>(function, list.mid(2));
            break;
        case ThermalReliefPad:
            m_function = std::make_shared<struct ThermalReliefPad>(function, list.mid(2));
            break;
        case WasherPad:
            m_function = std::make_shared<struct WasherPad>(function, list.mid(2));
            break;
        case AntiPad:
            m_function = std::make_shared<struct AntiPad>(function, list.mid(2));
            break;
        case OtherPad:
            m_function = std::make_shared<struct OtherPad>(function, list.mid(2));
            break;
        case Conductor:
            m_function = std::make_shared<struct Conductor>(function, list.mid(2));
            break;
        case EtchedComponent:
            m_function = std::make_shared<struct EtchedComponent>(function, list.mid(2));
            break;
        case NonConductor:
            m_function = std::make_shared<struct NonConductor>(function, list.mid(2));
            break;
        case CopperBalancing:
            m_function = std::make_shared<struct CopperBalancing>(function, list.mid(2));
            break;
        case Border:
            m_function = std::make_shared<struct Border>(function, list.mid(2));
            break;
        case OtherCopper:
            m_function = std::make_shared<struct OtherCopper>(function, list.mid(2));
            break;
        case ComponentMain: // Component
            m_function = std::make_shared<struct ComponentMain>(function, list.mid(2));
            break;
        case ComponentOutline:
            m_function = std::make_shared<struct ComponentOutline>(function, list.mid(2));
            break;
        case ComponentPin: //
            m_function = std::make_shared<struct ComponentPin>(function, list.mid(2));
            break;
        case Profile:
            m_function = std::make_shared<struct ProfileA>(function, list.mid(2));
            break;
        case NonMaterial:
            m_function = std::make_shared<struct NonMaterial>(function, list.mid(2));
            break;
        case Material:
            m_function = std::make_shared<struct Material>(function, list.mid(2));
            break;
        case Other:
            m_function = std::make_shared<struct OtherA>(function, list.mid(2));
            break;
        }
        break;
    case StdAttr::DrillTolerance:
        /*
        * <plus tolerance>,<minus tolerance>
        * %TA.DrillTolerance,0.01,0.005*%
        */
        m_drillTolerance = list;
        break;
    case StdAttr::FlashText:
        /*
        * <Text>,(B|C),[(R|M)],[<Font>],[Size],[<Comment>]
        * <text>
        * Текстовая строка, представленная изображением апертуры.
        * (B|C)
        * Указывает, представлен ли текст штрих-кодом - B - или символами - C.
        * (R|M)
        * Указывает, доступен ли текст для чтения или отображается влево-вправо. Необязательный.
        * <Font>
        * Название шрифта. Контент не стандартизирован. Необязательный.
        * <Size>
        * Размер шрифта. Контент не стандартизирован. Необязательный.
        * <Comments>
        * Любая дополнительная информация, которую вы хотите добавить. Необязательный.
        * Examples:
        * %TA.FlashText,L1,C,R,Courier,10,Layer number (L1 is top)*%
        * Text: L1
        * B|C: Characters,
        * (R|M): Readable
        * Font: Courier
        * Size: 10
        * Comment: Layer number (L1 is top)
        * %TA.FlashText,XZ12ADF,B,,Code128,,Project identifier *%
        * Text: XZ12ADF
        * B|C: Barcode
        * (R|M) Not specified
        * Font: Code128
        * Size: Not specified
        * Comment: Project identifier
        */
        qDebug() << (m_flashText = list);
        break;
    }
}

}
