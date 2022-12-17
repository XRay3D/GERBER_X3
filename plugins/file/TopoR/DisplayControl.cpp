#include "DisplayControl.h"

// namespace TopoR_PCB_Classes {

bool DisplayControl::Show_DisplayControl::getShowBoardOutlineSpecified() const {
    return _showBoardOutline != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowWiresSpecified() const {
    return _showWires != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowCoppersSpecified() const {
    return _showCoppers != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowTextsSpecified() const {
    return _showTexts != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getThroughPadSpecified() const {
    return _throughPad != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getThroughViaSpecified() const {
    return _throughVia != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getBurriedViaSpecified() const {
    return _burriedVia != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getBlindViaSpecified() const {
    return _blindVia != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getFixedViaSpecified() const {
    return _fixedVia != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowViasSpecified() const {
    return _showVias != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowSignalLayersSpecified() const {
    return _showSignalLayers != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowTopMechLayersSpecified() const {
    return _showTopMechLayers != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowBotMechLayersSpecified() const {
    return _showBotMechLayers != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowDocLayersSpecified() const {
    return _showDocLayers != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowTopMechDetailsSpecified() const {
    return _showTopMechDetails != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowBotMechDetailsSpecified() const {
    return _showBotMechDetails != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowMetalPadsSpecified() const {
    return _showMetalPads != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowTopMechPadsSpecified() const {
    return _showTopMechPads != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowBotMechPadsSpecified() const {
    return _showBotMechPads != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowNetLinesSpecified() const {
    return _showNetLines != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowMountingHolesSpecified() const {
    return _showMountingHoles != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowThinWiresSpecified() const {
    return _showThinWires != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowComponentsSpecified() const {
    return _showComponents != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowCompTopSpecified() const {
    return _showCompTop != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowCompBotSpecified() const {
    return _showCompBot != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowCompsDesSpecified() const {
    return _showCompsDes != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowPinsNameSpecified() const {
    return _showPinsName != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowPinsNetSpecified() const {
    return _showPinsNet != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowCompsBoundSpecified() const {
    return _showCompsBound != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowLabelRefDesSpecified() const {
    return _showLabelRefDes != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowLabelPartNameSpecified() const {
    return _showLabelPartName != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowLabelOtherSpecified() const {
    return _showLabelOther != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowViolationsSpecified() const {
    return _showViolations != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowNarrowSpecified() const {
    return _showNarrow != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowTrimmedSpecified() const {
    return _showTrimmed != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowDRCViolationsSpecified() const {
    return _showDRCViolations != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowKeepoutsSpecified() const {
    return _showKeepouts != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowRouteKeepoutsSpecified() const {
    return _showRouteKeepouts != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowPlaceKeepoutsSpecified() const {
    return _showPlaceKeepouts != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowActiveLayerOnlySpecified() const {
    return _showActiveLayerOnly != Bool::off;
}

bool DisplayControl::Show_DisplayControl::getShowSerpentAreaSpecified() const {
    return _showSerpentArea != Bool::off;
}

bool DisplayControl::Grid::getGridShowSpecified() const {
    return _gridShow != Bool::off;
}

bool DisplayControl::Grid::getAlignToGridSpecified() const {
    return _alignToGrid != Bool::off;
}

bool DisplayControl::Grid::getSnapToAngleSpecified() const {
    return _snapToAngle != Bool::off;
}

bool DisplayControl::LayerOptions::Show_LayerOptions::getVisibleSpecified() const {
    return _visible != Bool::off;
}

bool DisplayControl::LayerOptions::Show_LayerOptions::getDetailsSpecified() const {
    return _details != Bool::off;
}

bool DisplayControl::LayerOptions::Show_LayerOptions::getPadsSpecified() const {
    return _pads != Bool::off;
}

bool DisplayControl::ColorNets::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool DisplayControl::ColorNets::getColorizeWireSpecified() const {
    return _colorizeWire != Bool::off;
}

bool DisplayControl::ColorNets::getColorizePadSpecified() const {
    return _colorizePad != Bool::off;
}

bool DisplayControl::ColorNets::getColorizeCopperSpecified() const {
    return _colorizeCopper != Bool::off;
}

bool DisplayControl::ColorNets::getColorizeViaSpecified() const {
    return _colorizeVia != Bool::off;
}

bool DisplayControl::ColorNets::getColorizeNetlineSpecified() const {
    return _colorizeNetline != Bool::off;
}

bool DisplayControl::ColorNets::ShouldSerialize_SetColors() {
    return _SetColors.empty() ? false : _SetColors.size() > 0;
}

bool DisplayControl::FilterNetlines::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool DisplayControl::FilterNetlines::ShouldSerialize_Refs() {
    return _Refs.empty() ? false : _Refs.size() > 0;
}

bool DisplayControl::ShouldSerialize_LayersVisualOptions() {
    return _LayersVisualOptions.empty() ? false : _LayersVisualOptions.size() > 0;
}
// } // namespace TopoR_PCB_Classes
