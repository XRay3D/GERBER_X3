#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Настройки диалогов».
struct DialogSettings {
    // Настройки DRC.
    struct DRCSettings {
        // Настройка DRC: проверка зазоров.
        Xml::Attr<Bool> checkClearances;
        // Настройка DRC: проверка целостности цепей.
        Xml::Attr<Bool> checkNetIntegrity;
        // Настройка DRC: проверка ширины проводников.
        Xml::Attr<Bool> checkNetWidth;
        // Настройка DRC: проверять зазор между полигонами и краем платы.
        Xml::Attr<Bool> copperToBoard;
        // Настройка DRC: проверять зазор между полигонами.
        Xml::Attr<Bool> copperToCopper;
        // Настройка DRC: проверять зазор между полигонами и запретами.
        Xml::Attr<Bool> copperToKeepout;
        // Настройка DRC: проверять зазор между полигонами и контактными площадками.
        Xml::Attr<Bool> copperToPad;
        // Настройка DRC: проверять зазор между полигонами и переходными отверстиями.
        Xml::Attr<Bool> copperToVia;
        // Настройка DRC: проверять зазор между полигонами и проводниками.
        Xml::Attr<Bool> copperToWire;
        // Настройка DRC: выводить отчёт в указанный файл.
        Xml::Attr<Bool> createLog;
        // Настройка DRC: проверка зазоров между контактными площадками и краем платы.
        Xml::Attr<Bool> padToBoard;
        // Настройка DRC: проверка зазоров между контактными площадками и запретами.
        Xml::Attr<Bool> padToKeepout;
        // Настройка DRC: проверка зазоров между контактными площадками.
        Xml::Attr<Bool> padToPad;
        // Настройка DRC: проверять зазоры от надписей до края платы.
        Xml::Attr<Bool> textToBoard;
        // Настройка DRC: проверять зазоры между надписями и областями металлизации (полигонами).
        Xml::Attr<Bool> textToCopper;
        // Настройка DRC: проверять зазоры между надписями и запретами.
        Xml::Attr<Bool> textToKeepout;
        // Настройка DRC: проверять зазоры между надписями и контактными площадками.
        Xml::Attr<Bool> textToPad;
        // Настройка DRC: проверять зазоры между надписями и переходными отверстиями.
        Xml::Attr<Bool> textToVia;
        // Настройка DRC: проверять зазоры между надписями и проводниками.
        Xml::Attr<Bool> textToWire;
        // Настройка DRC: проверять зазоры от переходных отверстий до края платы.
        Xml::Attr<Bool> viaToBoard;
        // Настройка DRC: проверять зазоры между переходными отверстиями и запретами.
        Xml::Attr<Bool> viaToKeepout;
        // Настройка DRC: проверять зазоры между переходными отверстиями и контактными площадками.
        Xml::Attr<Bool> viaToPad;
        // Настройка DRC: проверять зазоры между переходными отверстиями.
        Xml::Attr<Bool> viaToVia;
        // Настройка DRC: проверять зазоры от проводников до края платы.
        Xml::Attr<Bool> wireToBoard;
        // Настройка DRC: проверять зазоры между проводниками и запретами.
        Xml::Attr<Bool> wireToKeepout;
        // Настройка DRC: проверять зазоры между проводниками и контактными площадками.
        Xml::Attr<Bool> wireToPad;
        // Настройка DRC: проверять зазоры между проводниками и переходными отверстиями.
        Xml::Attr<Bool> wireToVia;
        // Настройка DRC: проверять зазоры между проводниками.
        Xml::Attr<Bool> wireToWire;
        // Настройка DRC: файл для вывода отчета.
        Xml::Attr<QString> logFileName;
        // Настройка DRC: допуск.
        Xml::Attr<double> tolerance;
        // Настройка DRC: максимальное количество сообщений.
        Xml::Attr<int> messageLimit;
    };
    // Настройки вывода файлов Gerber.
    struct GerberSettings {
        // Настройки вывода файла Gerber.
        struct ExportFile {
            // Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            struct ExportObjects {
                // Настройка вывода файла Gerber: выводить контур платы.
                Xml::Attr<Bool> board;
                // Настройка вывода файлов Geber, DXF: выводить проводники.
                Xml::Attr<Bool> wires;
                // Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                Xml::Attr<Bool> coppers;
                // Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                Xml::Attr<Bool> padstacks;
                // Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                Xml::Attr<Bool> vias;
                // Настройка вывода файлов Gerber и DXF: выводить надписи.
                Xml::Attr<Bool> texts;
                // Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                Xml::Attr<Bool> labels;
                // Настройка вывода файлов Gerber: выводить детали на механических слоях.
                Xml::Attr<Bool> details;
                // Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                Xml::Attr<Bool> fiducials;
                /// \note !В TopoR реперные знаки не поддерживаются.</ remarks>
            };
            // Имя экспортируемого файла Gerber, Drill.
            /// \note !Имя не должно содержать путь к файлу.</ remarks>
            Xml::Attr<QString> fileName;
            // Настройка вывода файла Gerber: выводить файл.
            Xml::Attr<Bool> output;
            // Настройка вывода файла Gerber: вывод слоя в зеркальном отображении.
            Xml::Attr<Bool> mirror;
            // Настройка вывода файлов Gerber: инверсный вывод слоя.
            Xml::Attr<Bool> negative;
            // Ссылка на слой.
            LayerRef layerRef;
            // Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            ExportObjects exportObjects;
            // Настройка вывода файла Gerber: смещение объектов по осям x и y.
            Shift shift;
        };
        // Каталог для выходных файлов (Gerber, Drill).
        Xml::Attr<QString> outPath;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        Xml::Attr<units, NoOpt> units_;
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        Xml::Attr<int> intNums;
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        Xml::Attr<int> fractNums;
        // Настройки вывода файлов Gerber.
        Xml::Array<ExportFile> ExportFiles;
    };
    // Настройки вывода файла DXF.
    struct DXFSettings {
        // Настройки вывода слоя в файл DXF.
        struct ExportLayer {
            // Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            struct ExportObjects {
                // Настройка вывода файлов Geber, DXF: выводить проводники.
                Xml::Attr<Bool> wires;
                // Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                Xml::Attr<Bool> coppers;
                // Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                Xml::Attr<Bool> padstacks;
                // Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                Xml::Attr<Bool> vias;
                // Настройка вывода файлов Gerber и DXF: выводить надписи.
                Xml::Attr<Bool> texts;
                // Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                Xml::Attr<Bool> labels;
                // Настройка вывода файлов Gerber: выводить детали на механических слоях.
                Xml::Attr<Bool> details;
                // Настройка вывода слоя в файл DXF: выводить очертания компонентов.
                Xml::Attr<Bool> compsOutline;
                // Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                /// \note !В TopoR реперные знаки не поддерживаются.</ remarks>
                Xml::Attr<Bool> fiducials;
            };
            // Настройка вывода слоя в файл DXF: выводить слой.
            Xml::Attr<Bool> output;
            // Ссылка на слой.
            // [Xml::Element("LayerRef")] public LayerRef layerRef;
            LayerRef layerRef;
            // Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            ExportObjects exportObjects;
        };
        // Имя выходного файла (ВОМ, DXF).
        Xml::Attr<QString> outFile;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        Xml::Attr<units> units_;
        // Настройка вывода файла DXF: выводить слой с контуром платы.
        Xml::Attr<Bool> outputBoardLayer;
        // Настройка вывода файла DXF: выводить слой отверстий.
        Xml::Attr<Bool> outputDrillLayer;
        // Настройки вывода слоя в файл DXF.
        // [Xml::Element("ExportLayer")] public List<ExportLayer> ExportLayers_;
        Xml::ArrayElem<ExportLayer> ExportLayers;
    };

    // Настройки вывода файлов Drill.
    struct DrillSettings {
        // Настройки вывода файла Gerber.
        struct ExportFile {
            // Имя экспортируемого файла Gerber, Drill.
            /// \note !Имя не должно содержать путь к файлу.</ remarks>
            Xml::Attr<QString> fileName;
        };
        // Каталог для выходных файлов (Gerber, Drill).
        Xml::Attr<QString> outPath;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        Xml::Attr<units, NoOpt> units_;
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        Xml::Attr<int> intNums;
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        Xml::Attr<int> fractNums;
        // Настройки вывода файлов Gerber.
        // [Xml::Element("ExportFile")] public List<ExportFile_DrillSettings> ExportFiles_;
        Xml::Array<ExportFile> ExportFiles;
    };

    // Настройки вывода BOM файла.
    struct BOMSettings {
        // Имя выходного файла (ВОМ, DXF).
        Xml::Attr<QString, NoOpt> outFile;
        // Настройка диалога вывода BOM файла: выводить количество компонентов.
        Xml::Attr<Bool> count;
        // Настройка вывода BOM файла: выводить наименование компонентов.
        Xml::Attr<Bool> partName;
        // Настройка вывода BOM файла: выводить наименование посадочных мест.
        Xml::Attr<Bool> footprint;
        // Настройка вывода BOM файла: выводить позиционные обозначения компонентов.
        Xml::Attr<Bool> refDes;
        // Ссылка на атрибут.
        Xml::ArrayElem<AttributeRef> AttributeRefs;
    };

    // Настройка фильтра сообщений.
    struct MessageFilter {
        // Настройка фильтра сообщений: выводить сообщение 5003.
        Xml::Attr<Bool> W5003;
        // Настройка фильтра сообщений: выводить сообщение 5012.
        Xml::Attr<Bool> W5012;
        // Настройка фильтра сообщений: выводить сообщение 5013.
        Xml::Attr<Bool> W5013;
        // Настройка фильтра сообщений: выводить сообщение 5014.
        Xml::Attr<Bool> W5014;
        // Настройка фильтра сообщений: выводить сообщение 5015.
        Xml::Attr<Bool> W5015;
        // Настройка фильтра сообщений: выводить сообщение 5016.
        Xml::Attr<Bool> W5016;
        // Настройка фильтра сообщений: выводить сообщение 5017.
        Xml::Attr<Bool> W5017;
        // Настройка фильтра сообщений: выводить сообщение 5018.
        Xml::Attr<Bool> W5018;
        // Настройка фильтра сообщений: выводить сообщение 5023.
        Xml::Attr<Bool> W5023;
        // Настройка фильтра сообщений: выводить сообщение 5024.
        Xml::Attr<Bool> W5024;
        // Настройка фильтра сообщений: выводить сообщение 5026.
        Xml::Attr<Bool> W5026;
        // Настройка фильтра сообщений: выводить сообщение 5034.
        Xml::Attr<Bool> W5034;
        // Настройка фильтра сообщений: выводить сообщение 5036.
        Xml::Attr<Bool> W5036;
        // Настройка фильтра сообщений: выводить сообщение 5037.
        Xml::Attr<Bool> W5037;
        // Настройка фильтра сообщений: быстрая проверка зазоров между компонентами.
        Xml::Attr<Bool> WClrnBtwComps;
        // Настройка фильтра сообщений: быстрая проверка зазоров между объектами одной цепи.
        Xml::Attr<Bool> WClrnBtwObjSameNet;
        // Настройка фильтра сообщений: режим показа предупреждений.
        Xml::Attr<showWarnings> showWarnings_;
    };

    // Версия раздела.
    Xml::Attr<QString> version;
    // Настройки DRC.
    Xml::Optional<DRCSettings> dRCSettings;
    // Настройки вывода файлов Gerber.
    Xml::Optional<GerberSettings> gerberSettings;
    // Настройки вывода файла DXF.
    Xml::Optional<DXFSettings> dXFSettings;
    // Настройки вывода файлов Drill.
    Xml::Optional<DrillSettings> drillSettings;
    // Настройки вывода BOM файла.
    Xml::Optional<BOMSettings> bOMSettings;
    // Настройка фильтра сообщений.
    Xml::Optional<MessageFilter> messageFilter;
    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DialogSettings. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/
};

} // namespace TopoR
