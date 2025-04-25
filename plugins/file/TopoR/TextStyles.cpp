#include "TextStyles.h"

    // namespace TopoR_PCB_Classes {

    bool
    TextStyles::TextStyle::getBoldSpecified() const {
    return _bold != Bool::off;
}

bool TextStyles::TextStyle::getItalicSpecified() const {
    return _italic != Bool::off;
}

bool TextStyles::ShouldSerialize_TextStyles() {
    return _TextStyles.empty() ? false : _TextStyles.size() > 0;
}
// } // namespace TopoR_PCB_Classes
