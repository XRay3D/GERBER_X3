#pragma once

#include "Commons.h"
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе u"Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г."_s.
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
        // ORIGINAL LINE: [XmlAttribute(u"createLog"_s)] public Bool _createLog;
        Bool _createLog = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _createLogSpecified
        bool getCreateLogSpecified() const;

        /// <summary>
        /// Настройка DRC: файл для вывода отчета.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"logFileName"_s)] public string _logFileName;
        QString _logFileName;

        /// <summary>
        /// Настройка DRC: максимальное количество сообщений.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"messageLimit"_s, DataType = u"int"_s)] public int _messageLimit;
        int _messageLimit = 0;

        /// <summary>
        /// Настройка DRC: допуск.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"tolerance"_s, DataType = u"float"_s)] public float _tolerance;
        float _tolerance = 0.0F;

        /// <summary>
        /// Настройка DRC: проверка целостности цепей.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"checkNetIntegrity"_s)] public Bool _checkNetIntegrity;
        Bool _checkNetIntegrity = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _checkNetIntegritySpecified
        bool getCheckNetIntegritySpecified() const;

        /// <summary>
        /// Настройка DRC: проверка ширины проводников.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"checkNetWidth"_s)] public Bool _checkNetWidth;
        Bool _checkNetWidth = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _checkNetWidthSpecified
        bool getCheckNetWidthSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"checkClearances"_s)] public Bool _checkClearances;
        Bool _checkClearances = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _checkClearancesSpecified
        bool getCheckClearancesSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и областями металлизации (полигонами).
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"textToCopper"_s)] public Bool _textToCopper;
        Bool _textToCopper = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToCopperSpecified
        bool getTextToCopperSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"textToKeepout"_s)] public Bool _textToKeepout;
        Bool _textToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToKeepoutSpecified
        bool getTextToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"textToVia"_s)] public Bool _textToVia;
        Bool _textToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToViaSpecified
        bool getTextToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и проводниками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"textToWire"_s)] public Bool _textToWire;
        Bool _textToWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToWireSpecified
        bool getTextToWireSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между надписями и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"textToPad"_s)] public Bool _textToPad;
        Bool _textToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToPadSpecified
        bool getTextToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры от надписей до края платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"textToBoard"_s)] public Bool _textToBoard;
        Bool _textToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _textToBoardSpecified
        bool getTextToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"copperToCopper"_s)] public Bool _copperToCopper;
        Bool _copperToCopper = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToCopperSpecified
        bool getCopperToCopperSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"copperToKeepout"_s)] public Bool _copperToKeepout;
        Bool _copperToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToKeepoutSpecified
        bool getCopperToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и проводниками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"copperToWire"_s)] public Bool _copperToWire;
        Bool _copperToWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToWireSpecified
        bool getCopperToWireSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"copperToVia"_s)] public Bool _copperToVia;
        Bool _copperToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToViaSpecified
        bool getCopperToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"copperToPad"_s)] public Bool _copperToPad;
        Bool _copperToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToPadSpecified
        bool getCopperToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазор между полигонами и краем платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"copperToBoard"_s)] public Bool _copperToBoard;
        Bool _copperToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _copperToBoardSpecified
        bool getCopperToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"wireToKeepout"_s)] public Bool _wireToKeepout;
        Bool _wireToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToKeepoutSpecified
        bool getWireToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между переходными отверстиями и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"viaToKeepout"_s)] public Bool _viaToKeepout;
        Bool _viaToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToKeepoutSpecified
        bool getViaToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров между контактными площадками и запретами.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"padToKeepout"_s)] public Bool _padToKeepout;
        Bool _padToKeepout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _padToKeepoutSpecified
        bool getPadToKeepoutSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"wireToWire"_s)] public Bool _wireToWire;
        Bool _wireToWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToWireSpecified
        bool getWireToWireSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками и переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"wireToVia"_s)] public Bool _wireToVia;
        Bool _wireToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToViaSpecified
        bool getWireToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между проводниками и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"wireToPad"_s)] public Bool _wireToPad;
        Bool _wireToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToPadSpecified
        bool getWireToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры от проводников до края платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"wireToBoard"_s)] public Bool _wireToBoard;
        Bool _wireToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _wireToBoardSpecified
        bool getWireToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между переходными отверстиями.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"viaToVia"_s)] public Bool _viaToVia;
        Bool _viaToVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToViaSpecified
        bool getViaToViaSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры между переходными отверстиями и контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"viaToPad"_s)] public Bool _viaToPad;
        Bool _viaToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToPadSpecified
        bool getViaToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверять зазоры от переходных отверстий до края платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"viaToBoard"_s)] public Bool _viaToBoard;
        Bool _viaToBoard = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _viaToBoardSpecified
        bool getViaToBoardSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров между контактными площадками.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"padToPad"_s)] public Bool _padToPad;
        Bool _padToPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _padToPadSpecified
        bool getPadToPadSpecified() const;

        /// <summary>
        /// Настройка DRC: проверка зазоров между контактными площадками и краем платы.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"padToBoard"_s)] public Bool _padToBoard;
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
                // ORIGINAL LINE: [XmlAttribute(u"board"_s)] public Bool _board;
                Bool _board = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Geber, DXF: выводить проводники.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"wires"_s)] public Bool _wires;
                Bool _wires = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"coppers"_s)] public Bool _coppers;
                Bool _coppers = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"padstacks"_s)] public Bool _padstacks;
                Bool _padstacks = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"vias"_s)] public Bool _vias;
                Bool _vias = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber и DXF: выводить надписи.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"texts"_s)] public Bool _texts;
                Bool _texts = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"labels"_s)] public Bool _labels;
                Bool _labels = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber: выводить детали на механических слоях.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"details"_s)] public Bool _details;
                Bool _details = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                /// </summary
                /// <remarks>! В TopoR реперные знаки не поддерживаются.</remarks>

                // ORIGINAL LINE: [XmlAttribute(u"fiducials"_s)] public Bool _fiducials;
                Bool _fiducials = static_cast<Bool>(0);
            };

            /// <summary>
            /// Имя экспортируемого файла Gerber, Drill.
            /// </summary
            /// <remarks>! Имя не должно содержать путь к файлу.</remarks>
        public:
            // ORIGINAL LINE: [XmlAttribute(u"fileName"_s)] public string _fileName;
            QString _fileName;

            /// <summary>
            /// Настройка вывода файла Gerber: выводить файл.
            /// </summary

            // ORIGINAL LINE: [XmlAttribute(u"output"_s)] public Bool _output;
            Bool _output = static_cast<Bool>(0);

            /// <summary>
            /// Настройка вывода файла Gerber: вывод слоя в зеркальном отображении.
            /// </summary

            // ORIGINAL LINE: [XmlAttribute(u"mirror"_s)] public Bool _mirror;
            Bool _mirror = static_cast<Bool>(0);

            /// <summary>
            /// Настройка вывода файлов Gerber: инверсный вывод слоя.
            /// </summary

            // ORIGINAL LINE: [XmlAttribute(u"negative"_s)] public Bool _negative;
            Bool _negative = static_cast<Bool>(0);

            /// <summary>
            /// Ссылка на слой.
            /// </summary>

            // ORIGINAL LINE: [XmlElement(u"LayerRef"_s)] public LayerRef _LayerRef;
            LayerRef* _LayerRef;

            /// <summary>
            /// Настройка экспорта Gerber файлов: список экспортируемых объектов для слоя.
            /// </summary>

            // ORIGINAL LINE: [XmlElement(u"ExportObjects"_s)] public ExportObjects_ExportFile _ExportObjects;
            ExportObjects_ExportFile* _ExportObjects;

            /// <summary>
            /// Настройка вывода файла Gerber: смещение объектов по осям x и y.
            /// </summary>

            // ORIGINAL LINE: [XmlElement(u"Shift"_s)] public Shift _Shift;
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
        // ORIGINAL LINE: [XmlAttribute(u"outPath"_s)] public string _outPath;
        QString _outPath;

        /// <summary>
        /// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"units"_s)] public units _units;
        units _units = static_cast<units>(0);

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"intNums"_s, DataType = u"int"_s)] public int _intNums;
        int _intNums = 0;

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"fractNums"_s, DataType = u"int"_s)] public int _fractNums;
        int _fractNums = 0;

        /// <summary>
        /// Настройки вывода файлов Gerber.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"ExportFile"_s)] public List<ExportFile_GerberSettings> _ExportFiles;
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
                // ORIGINAL LINE: [XmlAttribute(u"wires"_s)] public Bool _wires;
                Bool _wires = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить области металлизации (полигоны).
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"coppers"_s)] public Bool _coppers;
                Bool _coppers = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить контактные площадки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"padstacks"_s)] public Bool _padstacks;
                Bool _padstacks = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить переходные отверстия.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"vias"_s)] public Bool _vias;
                Bool _vias = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber и DXF: выводить надписи.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"texts"_s)] public Bool _texts;
                Bool _texts = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить ярлыки.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"labels"_s)] public Bool _labels;
                Bool _labels = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber: выводить детали на механических слоях.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"details"_s)] public Bool _details;
                Bool _details = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода слоя в файл DXF: выводить очертания компонентов.
                /// </summary

                // ORIGINAL LINE: [XmlAttribute(u"compsOutline"_s)] public Bool _compsOutline;
                Bool _compsOutline = static_cast<Bool>(0);

                /// <summary>
                /// Настройка вывода файлов Gerber, DXF: выводить реперные знаки.
                /// </summary
                /// <remarks>! В TopoR реперные знаки не поддерживаются.</remarks>

                // ORIGINAL LINE: [XmlAttribute(u"fiducials"_s)] public Bool _fiducials;
                Bool _fiducials = static_cast<Bool>(0);
            };

            /// <summary>
            /// Настройка вывода слоя в файл DXF: выводить слой.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute(u"output"_s)] public Bool _output;
            Bool _output = static_cast<Bool>(0);

            /// <summary>
            /// Ссылка на слой.
            /// </summary>

            // ORIGINAL LINE: [XmlElement(u"LayerRef"_s)] public LayerRef _LayerRef;
            LayerRef* _LayerRef;

            /// <summary>
            /// Настройка экспорта слоя в файл DXF: список экспортируемых объектов для слоя.
            /// </summary>

            // ORIGINAL LINE: [XmlElement(u"ExportObjects"_s)] public ExportObjects_ExportLayer _ExportObjects;
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
        // ORIGINAL LINE: [XmlAttribute(u"outFile"_s)] public string _outFile;
        QString _outFile;

        /// <summary>
        /// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"units"_s)] public units _units;
        units _units = static_cast<units>(0);

        /// <summary>
        /// Настройка вывода файла DXF: выводить слой с контуром платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"outputBoardLayer"_s)] public Bool _outputBoardLayer;
        Bool _outputBoardLayer = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _outputBoardLayerSpecified
        bool getOutputBoardLayerSpecified() const;

        /// <summary>
        /// Настройка вывода файла DXF: выводить слой отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"outputDrillLayer"_s)] public Bool _outputDrillLayer;
        Bool _outputDrillLayer = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _outputDrillLayerSpecified
        bool getOutputDrillLayerSpecified() const;

        /// <summary>
        /// Настройки вывода слоя в файл DXF.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"ExportLayer"_s)] public List<ExportLayer> _ExportLayers;
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
            // ORIGINAL LINE: [XmlAttribute(u"fileName"_s)] public string _fileName;
            QString _fileName;
        };

        /// <summary>
        /// Каталог для выходных файлов (Gerber, Drill).
        /// </summary
    public:
        // ORIGINAL LINE: [XmlAttribute(u"outPath"_s)] public string _outPath;
        QString _outPath;

        /// <summary>
        /// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"units"_s)] public units _units;
        units _units = static_cast<units>(0);

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр перед запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"intNums"_s, DataType = u"int"_s)] public int _intNums;
        int _intNums = 0;

        /// <summary>
        /// Настройка вывода чисел в файлы Gerber, Drill: количество цифр после запятой.
        /// </summary

        // ORIGINAL LINE: [XmlAttribute(u"fractNums"_s, DataType = u"int"_s)] public int _fractNums;
        int _fractNums = 0;

        /// <summary>
        /// Настройки вывода файлов Gerber.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"ExportFile"_s)] public List<ExportFile_DrillSettings> _ExportFiles;
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
        // ORIGINAL LINE: [XmlAttribute(u"outFile"_s)] public string _outFile;
        QString _outFile;

        /// <summary>
        /// Настройка диалога вывода BOM файла: выводить количество компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"count"_s)] public Bool _count;
        Bool _count = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _countSpecified
        bool getCountSpecified() const;

        /// <summary>
        /// Настройка вывода BOM файла: выводить наименование компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"partName"_s)] public Bool _partName;
        Bool _partName = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _partNameSpecified
        bool getPartNameSpecified() const;

        /// <summary>
        /// Настройка вывода BOM файла: выводить наименование посадочных мест.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"footprint"_s)] public Bool _footprint;
        Bool _footprint = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _footprintSpecified
        bool getFootprintSpecified() const;

        /// <summary>
        /// Настройка вывода BOM файла: выводить позиционные обозначения компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"refDes"_s)] public Bool _refDes;
        Bool _refDes = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _refDesSpecified
        bool getRefDesSpecified() const;

        /// <summary>
        /// Ссылка на атрибут.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"AttributeRef"_s)] public List<AttributeRef> _AttributeRefs;
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
        // ORIGINAL LINE: [XmlAttribute(u"showWarnings"_s)] public showWarnings _showWarnings;
        showWarnings _showWarnings = static_cast<showWarnings>(0);

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5003.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5003"_s)] public Bool _W5003;
        Bool _W5003 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5003Specified
        bool getW5003Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5012.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5012"_s)] public Bool _W5012;
        Bool _W5012 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5012Specified
        bool getW5012Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5013.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5013"_s)] public Bool _W5013;
        Bool _W5013 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5013Specified
        bool getW5013Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5014.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5014"_s)] public Bool _W5014;
        Bool _W5014 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5014Specified
        bool getW5014Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5015.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5015"_s)] public Bool _W5015;
        Bool _W5015 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5015Specified
        bool getW5015Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5016.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5016"_s)] public Bool _W5016;
        Bool _W5016 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5016Specified
        bool getW5016Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5017.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5017"_s)] public Bool _W5017;
        Bool _W5017 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5017Specified
        bool getW5017Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5018.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5018"_s)] public Bool _W5018;
        Bool _W5018 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5018Specified
        bool getW5018Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5023.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5023"_s)] public Bool _W5023;
        Bool _W5023 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5023Specified
        bool getW5023Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5024.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5024"_s)] public Bool _W5024;
        Bool _W5024 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5024Specified
        bool getW5024Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5026.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5026"_s)] public Bool _W5026;
        Bool _W5026 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5026Specified
        bool getW5026Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5034.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5034"_s)] public Bool _W5034;
        Bool _W5034 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5034Specified
        bool getW5034Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5036.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5036"_s)] public Bool _W5036;
        Bool _W5036 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5036Specified
        bool getW5036Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: выводить сообщение 5037.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"W5037"_s)] public Bool _W5037;
        Bool _W5037 = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _W5037Specified
        bool getW5037Specified() const;

        /// <summary>
        /// Настройка фильтра сообщений: быстрая проверка зазоров между компонентами.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"WClrnBtwComps"_s)] public Bool _WClrnBtwComps;
        Bool _WClrnBtwComps = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _WClrnBtwCompsSpecified
        bool getWClrnBtwCompsSpecified() const;

        /// <summary>
        /// Настройка фильтра сообщений: быстрая проверка зазоров между объектами одной цепи.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"WClrnBtwObjSameNet"_s)] public Bool _WClrnBtwObjSameNet;
        Bool _WClrnBtwObjSameNet = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _WClrnBtwObjSameNetSpecified
        bool getWClrnBtwObjSameNetSpecified() const;
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute(u"version"_s)] public string _version;
    QString _version;

    /// <summary>
    /// Настройки DRC.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"DRCSettings"_s)] public DRCSettings _DRCSettings;
    DRCSettings* _DRCSettings;

    /// <summary>
    /// Настройки вывода файлов Gerber.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"GerberSettings"_s)] public GerberSettings _GerberSettings;
    GerberSettings* _GerberSettings;

    /// <summary>
    /// Настройки вывода файла DXF.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"DXFSettings"_s)] public DXFSettings _DXFSettings;
    DXFSettings* _DXFSettings;

    /// <summary>
    /// Настройки вывода файлов Drill.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"DrillSettings"_s)] public DrillSettings _DrillSettings;
    DrillSettings* _DrillSettings;

    /// <summary>
    /// Настройки вывода BOM файла.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"BOMSettings"_s)] public BOMSettings _BOMSettings;
    BOMSettings* _BOMSettings;

    /// <summary>
    /// Настройка фильтра сообщений.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"MessagesFilter"_s)] public MessagesFilter _MessagesFilter;
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
