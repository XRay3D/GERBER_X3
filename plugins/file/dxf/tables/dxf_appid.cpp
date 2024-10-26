// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
///*******************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  March 25, 2023                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2020                                          *
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
