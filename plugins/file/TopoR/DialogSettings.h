#pragma once

#include "Commons.h"
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Раздел «Настройки диалогов».
/// </summary>
class DialogSettings : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Настройки DRC.
    /// </summary>
public:
    class DRCSettings {
        /// <summary>
        /// Настройка DRC: выводить отчёт в указанный файл.
        /// </summary
    public:
        // ORIGINAL LINE: [XmlAttribute("createLog")] public Bool _createLog;
        Bool _createLog = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _createLogSpecified
        bool getCreateLogSpecified() const;

        /// <summary>
        /// Настройка DRC: файл для вывода отчета.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("logFileName")] public string _logFileName;
        QString _logFileName;

        /// <summary>
        /// Настройка DRC: максимальное количество сообщений.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("messageLimit", DataType = "int")] public int _messageLimit;
        int _messageLimit = 0;

        /// <summary>
        /// Настройка DRC: допуск.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("tolerance", DataType = "float")] public float _tolerance;
        float _tolerance = 0.0F;

        /// <summary>
        /// Настройка DRC: проверка целостности цепей.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("checkNetIntegrity")] public Bool _checkNetIntegrity;
        Bool _checkNetIntegrity = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _checkNetIntegritySpecified
        bool getCheckNetIntegritySpecified() const;

        /// <summary>
        /// Настройка DRC: проверка ширины проводников.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("checkNetWidth")] public Bool _checkNetWidth;
        Bool _checkNetWidth = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _checkNetWidthSpecified
        bool getCheckNetWidthSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("checkClearances")] public Bool _checkClearances;
        Bool _checkClearances = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _checkClearancesSpecified
        bool getCheckClearancesSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и областями металлизации (полигонами).
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("textToCopper")] public Bool _textToCopper;
        Bool _textToCopper = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToCopperSpecified
        bool getTextToCopperSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("textToKeepout")] public Bool _textToKeepout;
        Bool _textToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToKeepoutSpecified
        bool getTextToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("textToVia")] public Bool _textToVia;
        Bool _textToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToViaSpecified
        bool getTextToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и проводниками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("textToWire")] public Bool _textToWire;
        Bool _textToWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToWireSpecified
        bool getTextToWireSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("textToPad")] public Bool _textToPad;
        Bool _textToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToPadSpecified
        bool getTextToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры от надписей до края платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("textToBoard")] public Bool _textToBoard;
        Bool _textToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToBoardSpecified
        bool getTextToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("copperToCopper")] public Bool _copperToCopper;
        Bool _copperToCopper = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToCopperSpecified
        bool getCopperToCopperSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("copperToKeepout")] public Bool _copperToKeepout;
        Bool _copperToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToKeepoutSpecified
        bool getCopperToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и проводниками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("copperToWire")] public Bool _copperToWire;
        Bool _copperToWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToWireSpecified
        bool getCopperToWireSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("copperToVia")] public Bool _copperToVia;
        Bool _copperToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToViaSpecified
        bool getCopperToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("copperToPad")] public Bool _copperToPad;
        Bool _copperToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToPadSpecified
        bool getCopperToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и краем платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("copperToBoard")] public Bool _copperToBoard;
        Bool _copperToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToBoardSpecified
        bool getCopperToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("wireToKeepout")] public Bool _wireToKeepout;
        Bool _wireToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToKeepoutSpecified
        bool getWireToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между переходными отверстиями и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("viaToKeepout")] public Bool _viaToKeepout;
        Bool _viaToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToKeepoutSpecified
        bool getViaToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров между контактными площадками и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("padToKeepout")] public Bool _padToKeepout;
        Bool _padToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _padToKeepoutSpecified
        bool getPadToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("wireToWire")] public Bool _wireToWire;
        Bool _wireToWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToWireSpecified
        bool getWireToWireSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками и переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("wireToVia")] public Bool _wireToVia;
        Bool _wireToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToViaSpecified
        bool getWireToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("wireToPad")] public Bool _wireToPad;
        Bool _wireToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToPadSpecified
        bool getWireToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры от проводников до края платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("wireToBoard")] public Bool _wireToBoard;
        Bool _wireToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToBoardSpecified
        bool getWireToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("viaToVia")] public Bool _viaToVia;
        Bool _viaToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToViaSpecified
        bool getViaToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между переходными отверстиями и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("viaToPad")] public Bool _viaToPad;
        Bool _viaToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToPadSpecified
        bool getViaToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры от переходных отверстий до края платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("viaToBoard")] public Bool _viaToBoard;
        Bool _viaToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToBoardSpecified
        bool getViaToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров между контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("padToPad")] public Bool _padToPad;
        Bool _padToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _padToPadSpecified
        bool getPadToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров между контактными площадками и краем платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("padToBoard")] public Bool _padToBoard;
        Bool _padToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _padToBoardSpecified
        bool getPadToBoardSpecified() const;
    };

    /// <summary>
    /// Настройки вывода файлов Gerber.
    /// </summary>
public:
    class GerberSettings {
        /// <summary>
        /// Настройки вывода файла Gerber.
        /// </summary>
    public:
        class ExportFile_GerberSettings {
            /// <summary>
            /// Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            /// </summary>
        public:
            class ExportObjects_ExportFile {
                /// <summary>
                /// Настройка вывода файла Gerber: выводить контур платы.
                /// </summary
            public:
                // ORIGINAL LINE: [XmlAttribute("board")] public Bool _board;
                Bool _board = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Geber, DXF: выводить проводники.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("wires")] public Bool _wires;
                Bool _wires = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("coppers")] public Bool _coppers;
                Bool _coppers = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("padstacks")] public Bool _padstacks;
                Bool _padstacks = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("vias")] public Bool _vias;
                Bool _vias = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber и DXF: выводить надписи.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("texts")] public Bool _texts;
                Bool _texts = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("labels")] public Bool _labels;
                Bool _labels = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber: выводить детали на механических слоях.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("details")] public Bool _details;
                Bool _details = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                /// </summary
                /// <remarks>! В TopoR реперные знаки не поддерживаются.</remarks>

                // ORIGINAL LINE: [XmlAttribute("fiducials")] public Bool _fiducials;
                Bool _fiducials = static_cast<Bool>(0);
            };

            /// <summary>
            /// Имя экспортируемого файла Gerber, Drill.
            /// </summary
            /// <remarks>! Имя не должно содержать путь к файлу.</remarks>
        public:
            // ORIGINAL LINE: [XmlAttribute("fileName")] public string _fileName;
            QString _fileName;

            /// <summary>
            /// Настройка вывода файла Gerber: выводить файл.
            /// </summary

            // ORIGINAL LINE: [XmlAttribute("output")] public Bool _output;
            Bool _output = static_cast<Bool>(0);

            /// <summary>
            /// Настройка вывода файла Gerber: вывод слоя в зеркальном отображении.
            /// </summary

            // ORIGINAL LINE: [XmlAttribute("mirror")] public Bool _mirror;
            Bool _mirror = static_cast<Bool>(0);

            /// <summary>
            /// Настройка вывода файлов Gerber: инверсный вывод слоя.
            /// </summary

            // ORIGINAL LINE: [XmlAttribute("negative")] public Bool _negative;
            Bool _negative = static_cast<Bool>(0);

            /// <summary>
            /// Ссылка на слой.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
            LayerRef* _LayerRef;

            /// <summary>
            /// Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("ExportObjects")] public ExportObjects_ExportFile _ExportObjects;
            ExportObjects_ExportFile* _ExportObjects;

            /// <summary>
            /// Настройка вывода файла Gerber: смещение объектов по осям x и y.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("Shift")] public Shift _Shift;
            Shift* _Shift;
            virtual ~ExportFile_GerberSettings() {
                delete _LayerRef;
                delete _ExportObjects;
                delete _Shift;
            }
        };

        /// <summary>
        /// Каталог для выходных файлов (Gerber, Drill).
        /// </summary
    public:
        // ORIGINAL LINE: [XmlAttribute("outPath")] public string _outPath;
        QString _outPath;

        /// <summary>
        /// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("units")] public units _units;
        units _units = static_cast<units>(0);

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("intNums", DataType = "int")] public int _intNums;
        int _intNums = 0;

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("fractNums", DataType = "int")] public int _fractNums;
        int _fractNums = 0;

        /// <summary>
        /// Настройки вывода файлов Gerber.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("ExportFile")] public List<ExportFile_GerberSettings> _ExportFiles;
        std::vector<ExportFile_GerberSettings*> _ExportFiles;
        bool ShouldSerialize_ExportFiles();
    };

    /// <summary>
    /// Настройки вывода файла DXF.
    /// </summary>
public:
    class DXFSettings {
        /// <summary>
        /// Настройки вывода слоя в файл DXF.
        /// </summary>
    public:
        class ExportLayer {
            /// <summary>
            /// Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            /// </summary>
        public:
            class ExportObjects_ExportLayer {

                /// <summary>
                /// Настройка вывода файлов Geber, DXF: выводить проводники.
                /// </summary
            public:
                // ORIGINAL LINE: [XmlAttribute("wires")] public Bool _wires;
                Bool _wires = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("coppers")] public Bool _coppers;
                Bool _coppers = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("padstacks")] public Bool _padstacks;
                Bool _padstacks = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("vias")] public Bool _vias;
                Bool _vias = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber и DXF: выводить надписи.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("texts")] public Bool _texts;
                Bool _texts = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("labels")] public Bool _labels;
                Bool _labels = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber: выводить детали на механических слоях.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("details")] public Bool _details;
                Bool _details = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода слоя в файл DXF: выводить очертания компонентов.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute("compsOutline")] public Bool _compsOutline;
                Bool _compsOutline = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                /// </summary
                /// <remarks>! В TopoR реперные знаки не поддерживаются.</remarks>

                // ORIGINAL LINE: [XmlAttribute("fiducials")] public Bool _fiducials;
                Bool _fiducials = static_cast<Bool>(0);
            };

            /// <summary>
            /// Настройка вывода слоя в файл DXF: выводить слой.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("output")] public Bool _output;
            Bool _output = static_cast<Bool>(0);

            /// <summary>
            /// Ссылка на слой.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
            LayerRef* _LayerRef;

            /// <summary>
            /// Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("ExportObjects")] public ExportObjects_ExportLayer _ExportObjects;
            ExportObjects_ExportLayer* _ExportObjects;

            virtual ~ExportLayer() {
                delete _LayerRef;
                delete _ExportObjects;
            }
        };

        /// <summary>
        /// Имя выходного файла (ВОМ, DXF).
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("outFile")] public string _outFile;
        QString _outFile;

        /// <summary>
        /// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("units")] public units _units;
        units _units = static_cast<units>(0);

        /// <summary>
        /// Настройка вывода файла DXF: выводить слой с контуром платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("outputBoardLayer")] public Bool _outputBoardLayer;
        Bool _outputBoardLayer = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _outputBoardLayerSpecified
        bool getOutputBoardLayerSpecified() const;

        /// <summary>
        /// Настройка вывода файла DXF: выводить слой отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("outputDrillLayer")] public Bool _outputDrillLayer;
        Bool _outputDrillLayer = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _outputDrillLayerSpecified
        bool getOutputDrillLayerSpecified() const;

        /// <summary>
        /// Настройки вывода слоя в файл DXF.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("ExportLayer")] public List<ExportLayer> _ExportLayers;
        std::vector<ExportLayer*> _ExportLayers;
        bool ShouldSerialize_ExportLayers();
    };

    /// <summary>
    /// Настройки вывода файлов Drill.
    /// </summary>
public:
    class DrillSettings {
        /// <summary>
        /// Настройки вывода файла Gerber.
        /// </summary>
    public:
        class ExportFile_DrillSettings {
            /// <summary>
            /// Имя экспортируемого файла Gerber, Drill.
            /// </summary
            /// <remarks>! Имя не должно содержать путь к файлу.</remarks>
        public:
            // ORIGINAL LINE: [XmlAttribute("fileName")] public string _fileName;
            QString _fileName;
        };

        /// <summary>
        /// Каталог для выходных файлов (Gerber, Drill).
        /// </summary
    public:
        // ORIGINAL LINE: [XmlAttribute("outPath")] public string _outPath;
        QString _outPath;

        /// <summary>
        /// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("units")] public units _units;
        units _units = static_cast<units>(0);

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("intNums", DataType = "int")] public int _intNums;
        int _intNums = 0;

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute("fractNums", DataType = "int")] public int _fractNums;
        int _fractNums = 0;

        /// <summary>
        /// Настройки вывода файлов Gerber.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("ExportFile")] public List<ExportFile_DrillSettings> _ExportFiles;
        std::vector<ExportFile_DrillSettings*> _ExportFiles;
        bool ShouldSerialize_ExportFiles();
    };

    /// <summary>
    /// Настройки вывода BOM файла.
    /// </summary>
public:
    class BOMSettings {
        /// <summary>
        /// Имя выходного файла (ВОМ, DXF).
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("outFile")] public string _outFile;
        QString _outFile;

        /// <summary>
        /// Настройка диалога вывода BOM файла: выводить количество компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("count")] public Bool _count;
        Bool _count = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _countSpecified
        bool getCountSpecified() const;

        /// <summary>
        /// Настройка вывода BOM файла: выводить наименование компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("partName")] public Bool _partName;
        Bool _partName = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _partNameSpecified
        bool getPartNameSpecified() const;

        /// <summary>
        /// Настройка вывода BOM файла: выводить наименование посадочных мест.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("footprint")] public Bool _footprint;
        Bool _footprint = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _footprintSpecified
        bool getFootprintSpecified() const;

        /// <summary>
        /// Настройка вывода BOM файла: выводить позиционные обозначения компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("refDes")] public Bool _refDes;
        Bool _refDes = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _refDesSpecified
        bool getRefDesSpecified() const;

        /// <summary>
        /// Ссылка на атрибут.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("AttributeRef")] public List<AttributeRef> _AttributeRefs;
        std::vector<AttributeRef*> _AttributeRefs;
        bool ShouldSerialize_AttributeRefs();
    };

    /// <summary>
    /// Настройка фильтра сообщений.
    /// </summary>
public:
    class MessagesFilter {
        /// <summary>
        /// Настройка фильтра сообщений: режим показа предупреждений.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("showWarnings")] public showWarnings _showWarnings;
        showWarnings _showWarnings = static_cast<showWarnings>(0);

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5003.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5003")] public Bool _W5003;
        Bool _W5003 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5003Specified
        bool getW5003Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5012.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5012")] public Bool _W5012;
        Bool _W5012 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5012Specified
        bool getW5012Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5013.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5013")] public Bool _W5013;
        Bool _W5013 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5013Specified
        bool getW5013Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5014.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5014")] public Bool _W5014;
        Bool _W5014 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5014Specified
        bool getW5014Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5015.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5015")] public Bool _W5015;
        Bool _W5015 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5015Specified
        bool getW5015Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5016.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5016")] public Bool _W5016;
        Bool _W5016 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5016Specified
        bool getW5016Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5017.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5017")] public Bool _W5017;
        Bool _W5017 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5017Specified
        bool getW5017Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5018.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5018")] public Bool _W5018;
        Bool _W5018 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5018Specified
        bool getW5018Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5023.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5023")] public Bool _W5023;
        Bool _W5023 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5023Specified
        bool getW5023Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5024.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5024")] public Bool _W5024;
        Bool _W5024 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5024Specified
        bool getW5024Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5026.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5026")] public Bool _W5026;
        Bool _W5026 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5026Specified
        bool getW5026Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5034.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5034")] public Bool _W5034;
        Bool _W5034 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5034Specified
        bool getW5034Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5036.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5036")] public Bool _W5036;
        Bool _W5036 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5036Specified
        bool getW5036Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5037.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("W5037")] public Bool _W5037;
        Bool _W5037 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5037Specified
        bool getW5037Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: быстрая проверка зазоров между компонентами.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("WClrnBtwComps")] public Bool _WClrnBtwComps;
        Bool _WClrnBtwComps = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _WClrnBtwCompsSpecified
        bool getWClrnBtwCompsSpecified() const;

        /// <summary>
        /// Настройка фильтра сообщений: быстрая проверка зазоров между объектами одной цепи.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("WClrnBtwObjSameNet")] public Bool _WClrnBtwObjSameNet;
        Bool _WClrnBtwObjSameNet = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _WClrnBtwObjSameNetSpecified
        bool getWClrnBtwObjSameNetSpecified() const;
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Настройки DRC.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("DRCSettings")] public DRCSettings _DRCSettings;
    DRCSettings* _DRCSettings;

    /// <summary>
    /// Настройки вывода файлов Gerber.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("GerberSettings")] public GerberSettings _GerberSettings;
    GerberSettings* _GerberSettings;

    /// <summary>
    /// Настройки вывода файла DXF.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("DXFSettings")] public DXFSettings _DXFSettings;
    DXFSettings* _DXFSettings;

    /// <summary>
    /// Настройки вывода файлов Drill.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("DrillSettings")] public DrillSettings _DrillSettings;
    DrillSettings* _DrillSettings;

    /// <summary>
    /// Настройки вывода BOM файла.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("BOMSettings")] public BOMSettings _BOMSettings;
    BOMSettings* _BOMSettings;

    /// <summary>
    /// Настройка фильтра сообщений.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("MessagesFilter")] public MessagesFilter _MessagesFilter;
    MessagesFilter* _MessagesFilter;

    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DialogSettings. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/

    /**************************************************************************/

    virtual ~DialogSettings() {
        delete _DRCSettings;
        delete _GerberSettings;
        delete _DXFSettings;
        delete _DrillSettings;
        delete _BOMSettings;
        delete _MessagesFilter;
    }
};
// } // namespace TopoR_PCB_Classes
