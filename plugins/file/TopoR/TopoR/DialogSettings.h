#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Настройки диалогов».
struct DialogSettings {
    // Настройки DRC.
    struct DRCSettings {
        // Настройка DRC: выводить отчёт в указанный файл.
        [[= XML::Attr]] Bool createLog{};
        // public bool createLogSpecified

        // Настройка DRC: файл для вывода отчета.
        [[= XML::Attr]] std::string logFileName;
        // Настройка DRC: максимальное количество сообщений.
        [[= XML::Attr]] int messageLimit{};
        // Настройка DRC: допуск.
        [[= XML::Attr]] double tolerance{};
        // Настройка DRC: проверка целостности цепей.
        [[= XML::Attr]] Bool checkNetIntegrity{};
        // public bool checkNetIntegritySpecified

        // Настройка DRC: проверка ширины проводников.
        [[= XML::Attr]] Bool checkNetWidth{}; /// NOTE why skiped
        // public bool checkNetWidthSpecified

        // Настройка DRC: проверка зазоров.
        [[= XML::Attr]] Bool checkClearances{};
        // public bool checkClearancesSpecified

        // Настройка DRC: проверять зазоры между надписями и областями металлизации (полигонами).
        [[= XML::Attr]] Bool textToCopper{};
        // public bool textToCopperSpecified

        // Настройка DRC: проверять зазоры между надписями и запретами.
        [[= XML::Attr]] Bool textToKeepout{};
        // public bool textToKeepoutSpecified

        // Настройка DRC: проверять зазоры между надписями и переходными отверстиями.
        [[= XML::Attr]] Bool textToVia{};
        // public bool textToViaSpecified

        // Настройка DRC: проверять зазоры между надписями и проводниками.
        [[= XML::Attr]] Bool textToWire{};
        // public bool textToWireSpecified

        // Настройка DRC: проверять зазоры между надписями и контактными площадками.
        [[= XML::Attr]] Bool textToPad{};
        // public bool textToPadSpecified

        // Настройка DRC: проверять зазоры от надписей до края платы.
        [[= XML::Attr]] Bool textToBoard{};
        // public bool textToBoardSpecified

        // Настройка DRC: проверять зазор между полигонами.
        [[= XML::Attr]] Bool copperToCopper{};
        // public bool copperToCopperSpecified

        // Настройка DRC: проверять зазор между полигонами и запретами.
        [[= XML::Attr]] Bool copperToKeepout{};
        // public bool copperToKeepoutSpecified

        // Настройка DRC: проверять зазор между полигонами и проводниками.
        [[= XML::Attr]] Bool copperToWire{};
        // public bool copperToWireSpecified

        // Настройка DRC: проверять зазор между полигонами и переходными отверстиями.
        [[= XML::Attr]] Bool copperToVia{};
        // public bool copperToViaSpecified

        // Настройка DRC: проверять зазор между полигонами и контактными площадками.
        [[= XML::Attr]] Bool copperToPad{};
        // public bool copperToPadSpecified

        // Настройка DRC: проверять зазор между полигонами и краем платы.
        [[= XML::Attr]] Bool copperToBoard{};
        // public bool copperToBoardSpecified

        // Настройка DRC: проверять зазоры между проводниками и запретами.
        [[= XML::Attr]] Bool wireToKeepout{};
        // public bool wireToKeepoutSpecified

        // Настройка DRC: проверять зазоры между переходными отверстиями и запретами.
        [[= XML::Attr]] Bool viaToKeepout{};
        // public bool viaToKeepoutSpecified

        // Настройка DRC: проверка зазоров между контактными площадками и запретами.
        [[= XML::Attr]] Bool padToKeepout{};
        // public bool padToKeepoutSpecified

        // Настройка DRC: проверять зазоры между проводниками.
        [[= XML::Attr]] Bool wireToWire{};
        // public bool wireToWireSpecified

        // Настройка DRC: проверять зазоры между проводниками и переходными отверстиями.
        [[= XML::Attr]] Bool wireToVia{};
        // public bool wireToViaSpecified

        // Настройка DRC: проверять зазоры между проводниками и контактными площадками.
        [[= XML::Attr]] Bool wireToPad{};
        // public bool wireToPadSpecified

        // Настройка DRC: проверять зазоры от проводников до края платы.
        [[= XML::Attr]] Bool wireToBoard{};
        // public bool wireToBoardSpecified

        // Настройка DRC: проверять зазоры между переходными отверстиями.
        [[= XML::Attr]] Bool viaToVia{};
        // public bool viaToViaSpecified

        // Настройка DRC: проверять зазоры между переходными отверстиями и контактными площадками.
        [[= XML::Attr]] Bool viaToPad{};
        // public bool viaToPadSpecified

        // Настройка DRC: проверять зазоры от переходных отверстий до края платы.
        [[= XML::Attr]] Bool viaToBoard{};
        // public bool viaToBoardSpecified

        // Настройка DRC: проверка зазоров между контактными площадками.
        [[= XML::Attr]] Bool padToPad{};
        // public bool padToPadSpecified

        // Настройка DRC: проверка зазоров между контактными площадками и краем платы.
        [[= XML::Attr]] Bool padToBoard{};
        // public bool padToBoardSpecified
    };
    // Настройки вывода файлов Gerber.
    struct GerberSettings {
        // Настройки вывода файла Gerber.
        struct ExportFile {
            // Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            struct ExportObjects {
                // Настройка вывода файла Gerber: выводить контур платы.
                [[= XML::Attr]] Bool board{};
                // Настройка вывода файлов Geber, DXF: выводить проводники.
                [[= XML::Attr]] Bool wires{};
                // Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                [[= XML::Attr]] Bool coppers{};
                // Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                [[= XML::Attr]] Bool padstacks{};
                // Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                [[= XML::Attr]] Bool vias{};
                // Настройка вывода файлов Gerber и DXF: выводить надписи.
                [[= XML::Attr]] Bool texts{};
                // Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                [[= XML::Attr]] Bool labels{};
                // Настройка вывода файлов Gerber: выводить детали на механических слоях.
                [[= XML::Attr]] Bool details{};
                // Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                [[= XML::Attr]] Bool fiducials{};
            };
            // Имя экспортируемого файла Gerber, Drill.
            [[= XML::Attr]] std::string fileName;
            // Настройка вывода файла Gerber: выводить файл.
            [[= XML::Attr]] Bool output{};
            // Настройка вывода файла Gerber: вывод слоя в зеркальном отображении.
            [[= XML::Attr]] Bool mirror{};
            // Настройка вывода файлов Gerber: инверсный вывод слоя.
            [[= XML::Attr]] Bool negative{};
            // Ссылка на слой.
            // public LayerRef LayerRef;
            LayerRef LayerRef;
            // Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            ExportObjects ExportObjects;
            // Настройка вывода файла Gerber: смещение объектов по осям x и y.
            [[= XML::ElemF]] Shift Shift;
        };
        // Каталог для выходных файлов (Gerber, Drill).
        [[= XML::Attr]] std::string outPath;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        [[= XML::Attr]] units units{};
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        [[= XML::Attr]] int intNums{};
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        [[= XML::Attr]] int fractNums{};
        // Настройки вывода файлов Gerber.
        // public List<ExportFile_GerberSettings> ExportFiles;
        [[= XML::Elem]] std::vector<ExportFile> ExportFiles;
        bool ShouldSerialize_ExportFiles();
    };
    // Настройки вывода файла DXF.
    struct DXFSettings {
        // Настройки вывода слоя в файл DXF.
        struct ExportLayer {
            // Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            struct ExportObjects_ExportLayer {
                // Настройка вывода файлов Geber, DXF: выводить проводники.
                [[= XML::Attr]] Bool wires{};
                // Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                [[= XML::Attr]] Bool coppers{};
                // Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                [[= XML::Attr]] Bool padstacks{};
                // Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                [[= XML::Attr]] Bool vias{};
                // Настройка вывода файлов Gerber и DXF: выводить надписи.
                [[= XML::Attr]] Bool texts{};
                // Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                [[= XML::Attr]] Bool labels{};
                // Настройка вывода файлов Gerber: выводить детали на механических слоях.
                [[= XML::Attr]] Bool details{};
                // Настройка вывода слоя в файл DXF: выводить очертания компонентов.
                [[= XML::Attr]] Bool compsOutline{};
                // Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                [[= XML::Attr]] Bool fiducials{};
            };
            // Настройка вывода слоя в файл DXF: выводить слой.
            [[= XML::Attr]] Bool output{};
            // Ссылка на слой.
            // public LayerRef LayerRef;
            LayerRef LayerRef;
            // Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            // public ExportObjects_ExportLayer ExportObjects;
            ExportObjects_ExportLayer ExportObjects;
        };
        // Имя выходного файла (ВОМ, DXF).
        [[= XML::Attr]] std::string outFile;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        [[= XML::Attr]] units units{};
        // Настройка вывода файла DXF: выводить слой с контуром платы.
        [[= XML::Attr]] Bool outputBoardLayer{};
        // public bool outputBoardLayerSpecified

        // Настройка вывода файла DXF: выводить слой отверстий.
        [[= XML::Attr]] Bool outputDrillLayer{};
        // public bool outputDrillLayerSpecified

        // Настройки вывода слоя в файл DXF.
        // public List<ExportLayer> ExportLayers;
        [[= XML::Elem]] std::vector<ExportLayer> ExportLayers;
        bool ShouldSerialize_ExportLayers();
    };
    // Настройки вывода файлов Drill.
    struct DrillSettings {
        // Настройки вывода файла Gerber.
        struct ExportFile {
            // Имя экспортируемого файла Gerber, Drill.
            [[= XML::Attr]] std::string fileName;
        };
        // Каталог для выходных файлов (Gerber, Drill).
        [[= XML::Attr]] std::string outPath;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        [[= XML::Attr]] units units{};
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        [[= XML::Attr]] int intNums{};
        // Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        [[= XML::Attr]] int fractNums{};
        // Настройки вывода файлов Gerber.
        // public List<ExportFile_DrillSettings> ExportFiles;
        [[= XML::Elem]] std::vector<ExportFile> ExportFiles;
        bool ShouldSerialize_ExportFiles();
    };
    // Настройки вывода BOM файла.
    struct BOMSettings {
        // Имя выходного файла (ВОМ, DXF).
        [[= XML::Attr]] std::string outFile;
        // Настройка диалога вывода BOM файла: выводить количество компонентов.
        [[= XML::Attr]] Bool count{};
        // public bool countSpecified

        // Настройка вывода BOM файла: выводить наименование компонентов.
        [[= XML::Attr]] Bool partName{};
        // public bool partNameSpecified

        // Настройка вывода BOM файла: выводить наименование посадочных мест.
        [[= XML::Attr]] Bool footprint{};
        // public bool footprintSpecified

        // Настройка вывода BOM файла: выводить позиционные обозначения компонентов.
        [[= XML::Attr]] Bool refDes{};
        // public bool refDesSpecified

        // Ссылка на атрибут.
        // public List<AttributeRef> AttributeRefs;
        [[= XML::Elem]] std::vector<AttributeRef> AttributeRefs;
        bool ShouldSerialize_AttributeRefs();
    };
    // Настройка фильтра сообщений.
    struct MessageFilter {
        // Настройка фильтра сообщений: режим показа предупреждений.
        [[= XML::Attr]] showWarnings showWarnings{};
        // Настройка фильтра сообщений: выводить сообщение 5003.
        [[= XML::Attr]] Bool W5003{};
        // public bool W5003Specified

        // Настройка фильтра сообщений: выводить сообщение 5012.
        [[= XML::Attr]] Bool W5012{};
        // public bool W5012Specified

        // Настройка фильтра сообщений: выводить сообщение 5013.
        [[= XML::Attr]] Bool W5013{};
        // public bool W5013Specified

        // Настройка фильтра сообщений: выводить сообщение 5014.
        [[= XML::Attr]] Bool W5014{};
        // public bool W5014Specified

        // Настройка фильтра сообщений: выводить сообщение 5015.
        [[= XML::Attr]] Bool W5015{};
        // public bool W5015Specified

        // Настройка фильтра сообщений: выводить сообщение 5016.
        [[= XML::Attr]] Bool W5016{};
        // public bool W5016Specified

        // Настройка фильтра сообщений: выводить сообщение 5017.
        [[= XML::Attr]] Bool W5017{};
        // public bool W5017Specified

        // Настройка фильтра сообщений: выводить сообщение 5018.
        [[= XML::Attr]] Bool W5018{};
        // public bool W5018Specified

        // Настройка фильтра сообщений: выводить сообщение 5023.
        [[= XML::Attr]] Bool W5023{};
        // public bool W5023Specified

        // Настройка фильтра сообщений: выводить сообщение 5024.
        [[= XML::Attr]] Bool W5024{};
        // public bool W5024Specified

        // Настройка фильтра сообщений: выводить сообщение 5026.
        [[= XML::Attr]] Bool W5026{};
        // public bool W5026Specified

        // Настройка фильтра сообщений: выводить сообщение 5034.
        [[= XML::Attr]] Bool W5034{};
        // public bool W5034Specified

        // Настройка фильтра сообщений: выводить сообщение 5036.
        [[= XML::Attr]] Bool W5036{};
        // public bool W5036Specified

        // Настройка фильтра сообщений: выводить сообщение 5037.
        [[= XML::Attr]] Bool W5037{};
        // public bool W5037Specified

        // Настройка фильтра сообщений: быстрая проверка зазоров между компонентами.
        [[= XML::Attr]] Bool WClrnBtwComps{};
        // public bool WClrnBtwCompsSpecified

        // Настройка фильтра сообщений: быстрая проверка зазоров между объектами одной цепи.
        [[= XML::Attr]] Bool WClrnBtwObjSameNet{};
        // public bool WClrnBtwObjSameNetSpecified
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Настройки DRC.
    // public DRCSettings DRCSettings;
    DRCSettings DRCSettings;
    // Настройки вывода файлов Gerber.
    // public GerberSettings GerberSettings;
    GerberSettings GerberSettings;
    // Настройки вывода файла DXF.
    // public DXFSettings DXFSettings;
    DXFSettings DXFSettings;
    // Настройки вывода файлов Drill.
    // public DrillSettings DrillSettings;
    DrillSettings DrillSettings;
    // Настройки вывода BOM файла.
    // public BOMSettings BOMSettings;
    BOMSettings BOMSettings;
    // Настройка фильтра сообщений.
    // public MessagesFilter MessagesFilter;
    MessageFilter MessageFilter;
    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DialogSettings. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/
    /**************************************************************************/
};
} // namespace TopoR
