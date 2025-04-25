#include "DialogSettings.h"

    // namespace TopoR_PCB_Classes {

    bool
    DialogSettings::DRCSettings::getCreateLogSpecified() const {
    return _createLog != Bool::off;
}

bool DialogSettings::DRCSettings::getCheckNetIntegritySpecified() const {
    return _checkNetIntegrity != Bool::off;
}

bool DialogSettings::DRCSettings::getCheckNetWidthSpecified() const {
    return _checkNetWidth != Bool::off;
}

bool DialogSettings::DRCSettings::getCheckClearancesSpecified() const {
    return _checkClearances != Bool::off;
}

bool DialogSettings::DRCSettings::getTextToCopperSpecified() const {
    return _textToCopper != Bool::off;
}

bool DialogSettings::DRCSettings::getTextToKeepoutSpecified() const {
    return _textToKeepout != Bool::off;
}

bool DialogSettings::DRCSettings::getTextToViaSpecified() const {
    return _textToVia != Bool::off;
}

bool DialogSettings::DRCSettings::getTextToWireSpecified() const {
    return _textToWire != Bool::off;
}

bool DialogSettings::DRCSettings::getTextToPadSpecified() const {
    return _textToPad != Bool::off;
}

bool DialogSettings::DRCSettings::getTextToBoardSpecified() const {
    return _textToBoard != Bool::off;
}

bool DialogSettings::DRCSettings::getCopperToCopperSpecified() const {
    return _copperToCopper != Bool::off;
}

bool DialogSettings::DRCSettings::getCopperToKeepoutSpecified() const {
    return _copperToKeepout != Bool::off;
}

bool DialogSettings::DRCSettings::getCopperToWireSpecified() const {
    return _copperToWire != Bool::off;
}

bool DialogSettings::DRCSettings::getCopperToViaSpecified() const {
    return _copperToVia != Bool::off;
}

bool DialogSettings::DRCSettings::getCopperToPadSpecified() const {
    return _copperToPad != Bool::off;
}

bool DialogSettings::DRCSettings::getCopperToBoardSpecified() const {
    return _copperToBoard != Bool::off;
}

bool DialogSettings::DRCSettings::getWireToKeepoutSpecified() const {
    return _wireToKeepout != Bool::off;
}

bool DialogSettings::DRCSettings::getViaToKeepoutSpecified() const {
    return _viaToKeepout != Bool::off;
}

bool DialogSettings::DRCSettings::getPadToKeepoutSpecified() const {
    return _padToKeepout != Bool::off;
}

bool DialogSettings::DRCSettings::getWireToWireSpecified() const {
    return _wireToWire != Bool::off;
}

bool DialogSettings::DRCSettings::getWireToViaSpecified() const {
    return _wireToVia != Bool::off;
}

bool DialogSettings::DRCSettings::getWireToPadSpecified() const {
    return _wireToPad != Bool::off;
}

bool DialogSettings::DRCSettings::getWireToBoardSpecified() const {
    return _wireToBoard != Bool::off;
}

bool DialogSettings::DRCSettings::getViaToViaSpecified() const {
    return _viaToVia != Bool::off;
}

bool DialogSettings::DRCSettings::getViaToPadSpecified() const {
    return _viaToPad != Bool::off;
}

bool DialogSettings::DRCSettings::getViaToBoardSpecified() const {
    return _viaToBoard != Bool::off;
}

bool DialogSettings::DRCSettings::getPadToPadSpecified() const {
    return _padToPad != Bool::off;
}

bool DialogSettings::DRCSettings::getPadToBoardSpecified() const {
    return _padToBoard != Bool::off;
}

bool DialogSettings::GerberSettings::ShouldSerialize_ExportFiles() {
    return _ExportFiles.empty() ? false : _ExportFiles.size() > 0;
}

bool DialogSettings::DXFSettings::getOutputBoardLayerSpecified() const {
    return _outputBoardLayer != Bool::off;
}

bool DialogSettings::DXFSettings::getOutputDrillLayerSpecified() const {
    return _outputDrillLayer != Bool::off;
}

bool DialogSettings::DXFSettings::ShouldSerialize_ExportLayers() {
    return _ExportLayers.empty() ? false : _ExportLayers.size() > 0;
}

bool DialogSettings::DrillSettings::ShouldSerialize_ExportFiles() {
    return _ExportFiles.empty() ? false : _ExportFiles.size() > 0;
}

bool DialogSettings::BOMSettings::getCountSpecified() const {
    return _count != Bool::off;
}

bool DialogSettings::BOMSettings::getPartNameSpecified() const {
    return _partName != Bool::off;
}

bool DialogSettings::BOMSettings::getFootprintSpecified() const {
    return _footprint != Bool::off;
}

bool DialogSettings::BOMSettings::getRefDesSpecified() const {
    return _refDes != Bool::off;
}

bool DialogSettings::BOMSettings::ShouldSerialize_AttributeRefs() {
    return _AttributeRefs.empty() ? false : _AttributeRefs.size() > 0;
}

bool DialogSettings::MessagesFilter::getW5003Specified() const {
    return _W5003 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5012Specified() const {
    return _W5012 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5013Specified() const {
    return _W5013 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5014Specified() const {
    return _W5014 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5015Specified() const {
    return _W5015 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5016Specified() const {
    return _W5016 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5017Specified() const {
    return _W5017 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5018Specified() const {
    return _W5018 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5023Specified() const {
    return _W5023 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5024Specified() const {
    return _W5024 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5026Specified() const {
    return _W5026 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5034Specified() const {
    return _W5034 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5036Specified() const {
    return _W5036 != Bool::off;
}

bool DialogSettings::MessagesFilter::getW5037Specified() const {
    return _W5037 != Bool::off;
}

bool DialogSettings::MessagesFilter::getWClrnBtwCompsSpecified() const {
    return _WClrnBtwComps != Bool::off;
}

bool DialogSettings::MessagesFilter::getWClrnBtwObjSameNetSpecified() const {
    return _WClrnBtwObjSameNet != Bool::off;
}
// } // namespace TopoR_PCB_Classes
