#include "TextStyles.h"

#if TEXTSTYLES

namespace TopoR {

const TextStyles::TextStyle* TextStyles::getTextStyle(const QString& name) const {
    for(auto&& textStyle: textStyles)
        if(name == textStyle.name)
            return &textStyle;
    return {};
}


} // namespace TopoR

#endif
