///*******************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  XXXXX XX, 2025                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2025                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// *******************************************************************************/
// #include "dxf_appid.h"

// namespace Dxf {

// AppId::AppId(SectionParser* sp)
//     : TableItem{sp} {
// }

// void AppId::parse(CodeData& code) {
//     do {
//         data.push_back(code);
//         code = sp->nextCode();
//         switch (code.code()) {
//         case SubclassMarker:
//             break;
//         case ApplicationName:
//             applicationName = code;
//             break;
//         case StandardFlag:
//             standardFlag = code;
//             break;
//         }
//     } while (code.code() != 0);
// }

//} // namespace Dxf
