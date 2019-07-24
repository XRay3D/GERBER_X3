#include "edid.h"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#ifndef linux
#include <windows.h>

#include <initguid.h>

#include <Setupapi.h>

#define NAME_SIZE 128
#endif
QSizeF GetEdid()

{
    static QSizeF size;
    if (!size.isEmpty())
        return size;

#ifndef linux
    GUID GUID_CLASS_MONITOR = { 0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 };
    HDEVINFO devInfo = NULL;
    SP_DEVINFO_DATA devInfoData;
    do {
        devInfo = ::SetupDiGetClassDevsExA(
            &GUID_CLASS_MONITOR, // class GUID
            NULL, //                enumerator
            NULL, //                HWND
            DIGCF_PRESENT, //       Flags //DIGCF_ALLCLASSES|
            NULL, //                device info, create a new one.
            NULL, //                machine name, local machine
            NULL); //               reserved

        if (NULL == devInfo) {
            qDebug("SetupDiGetClassDevsEx");
            break;
        }

        for (ULONG i = 0; ERROR_NO_MORE_ITEMS != ::GetLastError(); i++) {
            memset(&devInfoData, 0, sizeof(devInfoData));
            devInfoData.cbSize = sizeof(devInfoData);
            if (::SetupDiEnumDeviceInfo(devInfo, i, &devInfoData)) {

                HKEY hDevRegKey;
                wchar_t uniID[123];

                if (::SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC /*SPDRP_UI_NUMBER*/, NULL, (PBYTE)(&uniID), sizeof(uniID), NULL)) {
                    qDebug() << "UID:" << QString::fromWCharArray(uniID);
                } else {
                    qDebug() << "ERROR:" << ::GetLastError();
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
                    qDebug() << "ERROR:" << GetLastError();
                }
            } // SetupDiEnumDeviceInfo
        } // for
    } while (0);
#endif
    if (size.isEmpty())
        size = QGuiApplication::screens()[0]->physicalSize();

    return size;
}
