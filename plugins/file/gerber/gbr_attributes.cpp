// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev *
 * Version   :  na           *
 * Date      :  01 February 2020              *
 * Website   :  na           *
 * Copyright :  Damir Bakiev 2016-2022        *
 * License: *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt       *
 *******************************************************************************/
#include "gbr_attributes.h"
#include "gbr_attraperfunction.h"
#include "gbr_attrfilefunction.h"

namespace Gerber::Attr {

File::StdAttr File::toStdAttr(const QString& key) {
    return static_cast<StdAttr>(
        staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()));
}

File::ePart File::toPart(const QString& key) {
    return static_cast<ePart>(
        staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().data()));
}

File::eFilePolarity File::toFilePolarityValue(const QString& key) {
    return static_cast<eFilePolarity>(
        staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().data()));
}

File::Function File::toFunction(const QString& key) {
    return static_cast<Function>(
        staticMetaObject.enumerator(3).keyToValue(key.toLocal8Bit().data()));
}

void File::parse(const QStringList& list) {
    switch (toStdAttr(list.first())) {
    case StdAttr::Part:
        part = list.mid(1);
        break;
    case StdAttr::FileFunction:
        switch (const auto function = toFunction(list[1]); function) {
        case Function::Legend:
            function_ = std::make_shared<struct Legend>(function, list.mid(2));
            break;
        case Function::Component:
            function_ = std::make_shared<struct Component>(function, list.mid(2));
            break;
        case Function::Heatsinkmask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::ArrayDrawing:
            function_ = std::make_shared<struct ArrayDrawing>(function, list.mid(2));
            break;
        case Function::AssemblyDrawing:
            function_ = std::make_shared<struct AssemblyDrawing>(function, list.mid(2));
            break;
        case Function::Carbonmask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Copper:
            function_ = std::make_shared<struct Copper>(function, list.mid(2));
            break;
        case Function::Depthrout:
            function_ = std::make_shared<struct Depthrout>(function, list.mid(2));
            break;
        case Function::Drillmap:
            function_ = std::make_shared<struct Drillmap>(function, list.mid(2));
            break;
        case Function::FabricationDrawing:
            function_ = std::make_shared<struct FabricationDrawing>(function, list.mid(2));
            break;
        case Function::Glue:
            function_ = std::make_shared<struct Glue>(function, list.mid(2));
            break;
        case Function::Goldmask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::NonPlated:
            function_ = std::make_shared<struct NonPlated>(function, list.mid(2));
            break;
        case Function::Other:
            function_ = std::make_shared<struct Other>(function, list.mid(2));
            break;
        case Function::OtherDrawing:
            function_ = std::make_shared<struct OtherDrawing>(function, list.mid(2));
            break;
        case Function::Pads:
            function_ = std::make_shared<struct Pads>(function, list.mid(2));
            break;
        case Function::Paste:
            function_ = std::make_shared<struct Paste>(function, list.mid(2));
            break;
        case Function::Peelablemask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Plated:
            function_ = std::make_shared<struct Plated>(function, list.mid(2));
            break;
        case Function::Profile:
            function_ = std::make_shared<struct Profile>(function, list.mid(2));
            break;
        case Function::Silvermask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Soldermask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Tinmask:
            function_ = std::make_shared<struct Mask>(function, list.mid(2));
            break;
        case Function::Vcut:
            function_ = std::make_shared<struct Vcut>(function, list.mid(2));
            break;
        case Function::Vcutmap:
            function_ = std::make_shared<struct Vcutmap>(function, list.mid(2));
            break;
        case Function::Viafill:
            function_ = std::make_shared<struct Viafill>(function, list.mid(2));
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
        // qDebug() << "custom" << custom;
    }
}
///////////////////////////////////////////////////////////////////////////////////
/// \brief Aperture::value
/// \param key
/// \return
///
int Aperture::value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }

Aperture::StdAttr Aperture::toStdAttr(const QString& key) {
    return static_cast<StdAttr>(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()));
}

Aperture::Function Aperture::toFunction(const QString& key) {
    return static_cast<Function>(staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().data()));
}

void Aperture::parse(const QStringList& list) {
    switch (toStdAttr(list.first())) {
    case StdAttr::AperFunction:
        // qDebug() << list;
        if (function_)
            function_.reset();
        switch (const auto function = toFunction(list[1]); function) {
        case ViaDrill:
            function_ = std::make_shared<struct ViaDrill>(function, list.mid(2));
            break;
        case BackDrill:
            function_ = std::make_shared<struct BackDrill>(function, list.mid(2));
            break;
        case ComponentDrill:
            function_ = std::make_shared<struct ComponentDrill>(function, list.mid(2));
            break;
        case MechanicalDrill:
            function_ = std::make_shared<struct MechanicalDrill>(function, list.mid(2));
            break;
        case CastellatedDrill:
            function_ = std::make_shared<struct CastellatedDrill>(function, list.mid(2));
            break;
        case OtherDrill:
            function_ = std::make_shared<struct OtherDrill>(function, list.mid(2));
            break;
        case ComponentPad:
            function_ = std::make_shared<struct ComponentPad>(function, list.mid(2));
            break;
        case SMDPad:
            function_ = std::make_shared<struct SMDPad>(function, list.mid(2));
            break;
        case BGAPad:
            function_ = std::make_shared<struct BGAPad>(function, list.mid(2));
            break;
        case ConnectorPad:
            function_ = std::make_shared<struct ConnectorPad>(function, list.mid(2));
            break;
        case HeatsinkPad:
            function_ = std::make_shared<struct HeatsinkPad>(function, list.mid(2));
            break;
        case ViaPad:
            function_ = std::make_shared<struct ViaPad>(function, list.mid(2));
            break;
        case TestPad:
            function_ = std::make_shared<struct TestPad>(function, list.mid(2));
            break;
        case CastellatedPad:
            function_ = std::make_shared<struct CastellatedPad>(function, list.mid(2));
            break;
        case FiducialPad:
            function_ = std::make_shared<struct FiducialPad>(function, list.mid(2));
            break;
        case ThermalReliefPad:
            function_ = std::make_shared<struct ThermalReliefPad>(function, list.mid(2));
            break;
        case WasherPad:
            function_ = std::make_shared<struct WasherPad>(function, list.mid(2));
            break;
        case AntiPad:
            function_ = std::make_shared<struct AntiPad>(function, list.mid(2));
            break;
        case OtherPad:
            function_ = std::make_shared<struct OtherPad>(function, list.mid(2));
            break;
        case Conductor:
            function_ = std::make_shared<struct Conductor>(function, list.mid(2));
            break;
        case EtchedComponent:
            function_ = std::make_shared<struct EtchedComponent>(function, list.mid(2));
            break;
        case NonConductor:
            function_ = std::make_shared<struct NonConductor>(function, list.mid(2));
            break;
        case CopperBalancing:
            function_ = std::make_shared<struct CopperBalancing>(function, list.mid(2));
            break;
        case Border:
            function_ = std::make_shared<struct Border>(function, list.mid(2));
            break;
        case OtherCopper:
            function_ = std::make_shared<struct OtherCopper>(function, list.mid(2));
            break;
        case ComponentMain: // Component
            function_ = std::make_shared<struct ComponentMain>(function, list.mid(2));
            break;
        case ComponentOutline:
            function_ = std::make_shared<struct ComponentOutline>(function, list.mid(2));
            break;
        case ComponentPin: //
            function_ = std::make_shared<struct ComponentPin>(function, list.mid(2));
            break;
        case Profile:
            function_ = std::make_shared<struct ProfileA>(function, list.mid(2));
            break;
        case NonMaterial:
            function_ = std::make_shared<struct NonMaterial>(function, list.mid(2));
            break;
        case Material:
            function_ = std::make_shared<struct Material>(function, list.mid(2));
            break;
        case Other:
            function_ = std::make_shared<struct OtherA>(function, list.mid(2));
            break;
        }
        break;
    case StdAttr::DrillTolerance:
        /*
         * <plus tolerance>,<minus tolerance>
         * %TA.DrillTolerance,0.01,0.005*%
         */
        drillTolerance_ = list;
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
        qDebug() << (flashText_ = list);
        break;
    }
}

} // namespace Gerber::Attr
