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
        bool getCreateLogSpecified() const;
        // Настройка DRC: файл для вывода отчета.
        [[= XML::Attr]] std::string logFileName;
        // Настройка DRC: максимальное количество сообщений.
        [[= XML::Attr]] int messageLimit{};
        // Настройка DRC: допуск.
        [[= XML::Attr]] double tolerance{};
        // Настройка DRC: проверка целостности цепей.
        [[= XML::Attr]] Bool checkNetIntegrity{};
        // public bool checkNetIntegritySpecified
        bool getCheckNetIntegritySpecified() const;
        // Настройка DRC: проверка ширины проводников.
        [[= XML::Attr]] Bool checkNetWidth{}; /// NOTE why skiped
        // public bool checkNetWidthSpecified
        bool getCheckNetWidthSpecified() const;
        // Настройка DRC: проверка зазоров.
        [[= XML::Attr]] Bool checkClearances{};
        // public bool checkClearancesSpecified
        bool getCheckClearancesSpecified() const;
        // Настройка DRC: проверять зазоры между надписями и областями металлизации (полигонами).
        [[= XML::Attr]] Bool textToCopper{};
        // public bool textToCopperSpecified
        bool getTextToCopperSpecified() const;
        // Настройка DRC: проверять зазоры между надписями и запретами.
        [[= XML::Attr]] Bool textToKeepout{};
        // public bool textToKeepoutSpecified
        bool getTextToKeepoutSpecified() const;
        // Настройка DRC: проверять зазоры между надписями и переходными отверстиями.
        [[= XML::Attr]] Bool textToVia{};
        // public bool textToViaSpecified
        bool getTextToViaSpecified() const;
        // Настройка DRC: проверять зазоры между надписями и проводниками.
        [[= XML::Attr]] Bool textToWire{};
        // public bool textToWireSpecified
        bool getTextToWireSpecified() const;
        // Настройка DRC: проверять зазоры между надписями и контактными площадками.
        [[= XML::Attr]] Bool textToPad{};
        // public bool textToPadSpecified
        bool getTextToPadSpecified() const;
        // Настройка DRC: проверять зазоры от надписей до края платы.
        [[= XML::Attr]] Bool textToBoard{};
        // public bool textToBoardSpecified
        bool getTextToBoardSpecified() const;
        // Настройка DRC: проверять зазор между полигонами.
        [[= XML::Attr]] Bool copperToCopper{};
        // public bool copperToCopperSpecified
        bool getCopperToCopperSpecified() const;
        // Настройка DRC: проверять зазор между полигонами и запретами.
        [[= XML::Attr]] Bool copperToKeepout{};
        // public bool copperToKeepoutSpecified
        bool getCopperToKeepoutSpecified() const;
        // Настройка DRC: проверять зазор между полигонами и проводниками.
        [[= XML::Attr]] Bool copperToWire{};
        // public bool copperToWireSpecified
        bool getCopperToWireSpecified() const;
        // Настройка DRC: проверять зазор между полигонами и переходными отверстиями.
        [[= XML::Attr]] Bool copperToVia{};
        // public bool copperToViaSpecified
        bool getCopperToViaSpecified() const;
        // Настройка DRC: проверять зазор между полигонами и контактными площадками.
        [[= XML::Attr]] Bool copperToPad{};
        // public bool copperToPadSpecified
        bool getCopperToPadSpecified() const;
        // Настройка DRC: проверять зазор между полигонами и краем платы.
        [[= XML::Attr]] Bool copperToBoard{};
        // public bool copperToBoardSpecified
        bool getCopperToBoardSpecified() const;
        // Настройка DRC: проверять зазоры между проводниками и запретами.
        [[= XML::Attr]] Bool wireToKeepout{};
        // public bool wireToKeepoutSpecified
        bool getWireToKeepoutSpecified() const;
        // Настройка DRC: проверять зазоры между переходными отверстиями и запретами.
        [[= XML::Attr]] Bool viaToKeepout{};
        // public bool viaToKeepoutSpecified
        bool getViaToKeepoutSpecified() const;
        // Настройка DRC: проверка зазоров между контактными площадками и запретами.
        [[= XML::Attr]] Bool padToKeepout{};
        // public bool padToKeepoutSpecified
        bool getPadToKeepoutSpecified() const;
        // Настройка DRC: проверять зазоры между проводниками.
        [[= XML::Attr]] Bool wireToWire{};
        // public bool wireToWireSpecified
        bool getWireToWireSpecified() const;
        // Настройка DRC: проверять зазоры между проводниками и переходными отверстиями.
        [[= XML::Attr]] Bool wireToVia{};
        // public bool wireToViaSpecified
        bool getWireToViaSpecified() const;
        // Настройка DRC: проверять зазоры между проводниками и контактными площадками.
        [[= XML::Attr]] Bool wireToPad{};
        // public bool wireToPadSpecified
        bool getWireToPadSpecified() const;
        // Настройка DRC: проверять зазоры от проводников до края платы.
        [[= XML::Attr]] Bool wireToBoard{};
        // public bool wireToBoardSpecified
        bool getWireToBoardSpecified() const;
        // Настройка DRC: проверять зазоры между переходными отверстиями.
        [[= XML::Attr]] Bool viaToVia{};
        // public bool viaToViaSpecified
        bool getViaToViaSpecified() const;
        // Настройка DRC: проверять зазоры между переходными отверстиями и контактными площадками.
        [[= XML::Attr]] Bool viaToPad{};
        // public bool viaToPadSpecified
        bool getViaToPadSpecified() const;
        // Настройка DRC: проверять зазоры от переходных отверстий до края платы.
        [[= XML::Attr]] Bool viaToBoard{};
        // public bool viaToBoardSpecified
        bool getViaToBoardSpecified() const;
        // Настройка DRC: проверка зазоров между контактными площадками.
        [[= XML::Attr]] Bool padToPad{};
        // public bool padToPadSpecified
        bool getPadToPadSpecified() const;
        // Настройка DRC: проверка зазоров между контактными площадками и краем платы.
        [[= XML::Attr]] Bool padToBoard{};
        // public bool padToBoardSpecified
        bool getPadToBoardSpecified() const;
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
            /*[[= XML::Elem]]*/ LayerRef LayerRef;
            // Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            /*[[= XML::Elem]]*/ ExportObjects ExportObjects;
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
            /*[[= XML::Elem]]*/ LayerRef LayerRef;
            // Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            // public ExportObjects_ExportLayer ExportObjects;
            /*[[= XML::Elem]]*/ ExportObjects_ExportLayer ExportObjects;
        };
        // Имя выходного файла (ВОМ, DXF).
        [[= XML::Attr]] std::string outFile;
        // Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        [[= XML::Attr]] units units{};
        // Настройка вывода файла DXF: выводить слой с контуром платы.
        [[= XML::Attr]] Bool outputBoardLayer{};
        // public bool outputBoardLayerSpecified
        bool getOutputBoardLayerSpecified() const;
        // Настройка вывода файла DXF: выводить слой отверстий.
        [[= XML::Attr]] Bool outputDrillLayer{};
        // public bool outputDrillLayerSpecified
        bool getOutputDrillLayerSpecified() const;
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
        bool getCountSpecified() const;
        // Настройка вывода BOM файла: выводить наименование компонентов.
        [[= XML::Attr]] Bool partName{};
        // public bool partNameSpecified
        bool getPartNameSpecified() const;
        // Настройка вывода BOM файла: выводить наименование посадочных мест.
        [[= XML::Attr]] Bool footprint{};
        // public bool footprintSpecified
        bool getFootprintSpecified() const;
        // Настройка вывода BOM файла: выводить позиционные обозначения компонентов.
        [[= XML::Attr]] Bool refDes{};
        // public bool refDesSpecified
        bool getRefDesSpecified() const;
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
        bool getW5003Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5012.
        [[= XML::Attr]] Bool W5012{};
        // public bool W5012Specified
        bool getW5012Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5013.
        [[= XML::Attr]] Bool W5013{};
        // public bool W5013Specified
        bool getW5013Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5014.
        [[= XML::Attr]] Bool W5014{};
        // public bool W5014Specified
        bool getW5014Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5015.
        [[= XML::Attr]] Bool W5015{};
        // public bool W5015Specified
        bool getW5015Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5016.
        [[= XML::Attr]] Bool W5016{};
        // public bool W5016Specified
        bool getW5016Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5017.
        [[= XML::Attr]] Bool W5017{};
        // public bool W5017Specified
        bool getW5017Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5018.
        [[= XML::Attr]] Bool W5018{};
        // public bool W5018Specified
        bool getW5018Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5023.
        [[= XML::Attr]] Bool W5023{};
        // public bool W5023Specified
        bool getW5023Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5024.
        [[= XML::Attr]] Bool W5024{};
        // public bool W5024Specified
        bool getW5024Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5026.
        [[= XML::Attr]] Bool W5026{};
        // public bool W5026Specified
        bool getW5026Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5034.
        [[= XML::Attr]] Bool W5034{};
        // public bool W5034Specified
        bool getW5034Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5036.
        [[= XML::Attr]] Bool W5036{};
        // public bool W5036Specified
        bool getW5036Specified() const;
        // Настройка фильтра сообщений: выводить сообщение 5037.
        [[= XML::Attr]] Bool W5037{};
        // public bool W5037Specified
        bool getW5037Specified() const;
        // Настройка фильтра сообщений: быстрая проверка зазоров между компонентами.
        [[= XML::Attr]] Bool WClrnBtwComps{};
        // public bool WClrnBtwCompsSpecified
        bool getWClrnBtwCompsSpecified() const;
        // Настройка фильтра сообщений: быстрая проверка зазоров между объектами одной цепи.
        [[= XML::Attr]] Bool WClrnBtwObjSameNet{};
        // public bool WClrnBtwObjSameNetSpecified
        bool getWClrnBtwObjSameNetSpecified() const;
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Настройки DRC.
    // public DRCSettings DRCSettings;
    /*[[= XML::Elem]]*/ DRCSettings DRCSettings;
    // Настройки вывода файлов Gerber.
    // public GerberSettings GerberSettings;
    /*[[= XML::Elem]]*/ GerberSettings GerberSettings;
    // Настройки вывода файла DXF.
    // public DXFSettings DXFSettings;
    /*[[= XML::Elem]]*/ DXFSettings DXFSettings;
    // Настройки вывода файлов Drill.
    // public DrillSettings DrillSettings;
    /*[[= XML::Elem]]*/ DrillSettings DrillSettings;
    // Настройки вывода BOM файла.
    // public BOMSettings BOMSettings;
    /*[[= XML::Elem]]*/ BOMSettings BOMSettings;
    // Настройка фильтра сообщений.
    // public MessagesFilter MessagesFilter;
    /*[[= XML::Elem]]*/ MessageFilter MessageFilter;
    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DialogSettings. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/
    /**************************************************************************/
};
} // namespace TopoR
