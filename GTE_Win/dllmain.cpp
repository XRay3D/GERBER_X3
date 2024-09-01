// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Bakiev Damir                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Bakiev Damir 2010-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/

#include "ClassFactory.h" // For the class factory
#include "Reg.h"
#include <QApplication>
#include <ShlObj.h> // For SHChangeNotify
#include <Windows.h>
#include <guiddef.h>

// {4D2FBA8D-621B-4447-AF6D-5794F479C4A5}
// When you write your own handler, you must create a new CLSID by using the
// "Create GUID" tool in the Tools menu, and specify the CLSID value here.
const CLSID CLSID_GerberThumbnailProvider = {
    0x4D2FB08D, 0x621B, 0x4447, {0xAF, 0x6D, 0x17, 0x94, 0xF4, 0x79, 0xC4, 0xA5}
};

static HINSTANCE g_hInst = nullptr;
static QApplication* app = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID /*lpReserved*/) {
    int c = 0;
    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        // Hold the instance of this DLL module, we will use it to get the
        // path of the DLL to register the component.
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        app = new QApplication(c, static_cast<char**>(nullptr), 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if(app) {
            delete app;
            app = nullptr;
        }
        break;
    }
    return TRUE;
}

//
//   FUNCTION: DllGetClassObject
//
//   PURPOSE: Create the class factory and query to the specific interface.
//
//   PARAMETERS:
//   * rclsid - The CLSID that will associate the correct data and code.
//   * riid - A reference to the identifier of the interface that the caller
//     is to use to communicate with the class object.
//   * ppv - The address of a pointer variable that receives the interface
//     pointer requested in riid. Upon successful return, *ppv contains the
//     requested interface pointer. If an error occurs, the interface pointer
//     is NULL.
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if(IsEqualCLSID(CLSID_GerberThumbnailProvider, rclsid)) {
        hr = E_OUTOFMEMORY;

        ClassFactory* pClassFactory = new ClassFactory();
        if(pClassFactory) {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}

//
//   FUNCTION: DllCanUnloadNow
//
//   PURPOSE: Check if we can unload the component from the memory.
//
//   NOTE: The component can be unloaded from the memory when its reference
//   count is zero (i.e. nobody is still using the component).
//
extern long g_cDllRef;
STDAPI DllCanUnloadNow(void) {
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}

//
//   FUNCTION: DllRegisterServer
//
//   PURPOSE: Register the COM server and the thumbnail handler.
//
static const QString fileFormats(".GBR|.GBL|.GBO|.GBP|.GBS|.GML|.GTL|.GTO|.GTP|.GTS|.BRD|.ART|.PHO");

STDAPI DllRegisterServer(void) {
    HRESULT hr;

    wchar_t szModule[MAX_PATH];
    if(GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Register the component.
    hr = RegisterInprocServer(szModule, CLSID_GerberThumbnailProvider,
        L"GerberThumbnailExtension.GerberThumbnailProvider Class",
        L"Apartment");
    if(SUCCEEDED(hr)) {
        // Register the thumbnail handler. The thumbnail handler is associated
        // with the .gbr file class.

        for(QString& var: fileFormats.split('|')) {
            hr = RegisterShellExtThumbnailHandler(var.toStdWString().data(), CLSID_GerberThumbnailProvider);
            if(SUCCEEDED(hr)) {
                // This tells the shell to invalidate the thumbnail cache. It is
                // important because any .gbr files viewed before registering
                // this handler would otherwise show cached blank thumbnails.
                SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
            }
        }
    }

    return hr;
}

//
//   FUNCTION: DllUnregisterServer
//
//   PURPOSE: Unregister the COM server and the thumbnail handler.
//
STDAPI DllUnregisterServer(void) {
    HRESULT hr = S_OK;

    wchar_t szModule[MAX_PATH];
    if(GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Unregister the component.
    hr = UnregisterInprocServer(CLSID_GerberThumbnailProvider);

    if(SUCCEEDED(hr)) {
        // Unregister the thumbnail handler.
        for(QString& var: fileFormats.split('|'))
            hr = UnregisterShellExtThumbnailHandler(var.toStdWString().data());
    }

    return hr;
}
