// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "edid.h"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#ifndef linux
#include "initguid.h"
#include "windows.h"
#include <Setupapi.h>
#define NAME_SIZE 128
#pragma comment(lib, "Setupapi")
#endif

QSizeF GetRealSize() {
    static QSizeF size;
    if(!size.isEmpty())
        return size;

#if 0 // ndef linux
    GUID GUID_CLASS_MONITOR = { 0x4d36e96e, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };
    HDEVINFO devInfo = nullptr;
    SP_DEVINFO_DATA devInfoData;
    do {
        devInfo = ::SetupDiGetClassDevsExA(
            &GUID_CLASS_MONITOR, // class GUID
            nullptr, //                enumerator
            nullptr, //                HWND
            DIGCF_PRESENT, //       Flags //DIGCF_ALLCLASSES|
            nullptr, //                device info, create a new one.
            nullptr, //                machine name, local machine
            nullptr); //               reserved

        if (nullptr == devInfo) {
            break;
        }

        for (ULONG i = 0; ERROR_NO_MORE_ITEMS != ::GetLastError(); i++) {
            memset(&devInfoData, 0, sizeof(devInfoData));
            devInfoData.cbSize = sizeof(devInfoData);
            if (::SetupDiEnumDeviceInfo(devInfo, i, &devInfoData)) {

                HKEY hDevRegKey;
                wchar_t uniID[123];

                if (!::SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC /*SPDRP_UI_NUMBER*/, NULL, (PBYTE)(&uniID), sizeof(uniID), NULL)) {
                    break;
                }

                hDevRegKey = ::SetupDiOpenDevRegKey(devInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ /*KEY_ALL_ACCESS*/);

                if (hDevRegKey) {
                    LONG retValue, i;
                    DWORD dwType, AcutalValueNameLength = NAME_SIZE;

                    CHAR valueName[NAME_SIZE];

                    for (i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; i++) {
                        uchar EDIDdata[1024];
                        DWORD edidsize = 1024;

                        retValue = RegEnumValueA(hDevRegKey, i, valueName, &AcutalValueNameLength, NULL /*reserved*/, &dwType, EDIDdata /*buffer*/, &edidsize /*buffer size*/);

                        if (retValue == ERROR_SUCCESS) {
                            if (!strcmp(valueName, "EDID")) {
                                size = QSizeF(((EDIDdata[68] & 0xF0) << 4) | EDIDdata[66], ((EDIDdata[68] & 0x0F) << 8) | EDIDdata[67]);
                                break;
                            }
                        }
                    }
                    RegCloseKey(hDevRegKey);
                } else {
                }
            } // SetupDiEnumDeviceInfo
        } // for
    } while (0);
#endif
    if(size.isEmpty()) // FIXME current display of graficsview
        size = QGuiApplication::screens()[0]->physicalSize();

    return size;
}
