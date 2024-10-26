// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "ClassFactory.h"
#include "gerberthumbnailprovider.h"
#include <Shlwapi.h>
#include <new>
#pragma comment(lib, "shlwapi.lib")

long g_cDllRef = 0;

ClassFactory::ClassFactory()
    : m_cRef(1) {
    InterlockedIncrement(&g_cDllRef);
}

ClassFactory::~ClassFactory() { InterlockedDecrement(&g_cDllRef); }

//
// IUnknown
//
IFACEMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(ClassFactory, IClassFactory),
        {nullptr, 0}
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG)
ClassFactory::AddRef() {
    return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG)
ClassFactory::Release() {
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if(0 == cRef)
        delete this;
    return cRef;
}

//
// IClassFactory
//

IFACEMETHODIMP ClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) {
    HRESULT hr = CLASS_E_NOAGGREGATION;

    // pUnkOuter is used for aggregation. We do not support it in the sample.
    if(pUnkOuter == nullptr) {
        hr = E_OUTOFMEMORY;

        // Create the COM component.
        GerberThumbnailProvider* pExt = new(std::nothrow) GerberThumbnailProvider();
        if(pExt) {
            // Query the specified interface.
            hr = pExt->QueryInterface(riid, ppv);
            pExt->Release();
        }
    }

    return hr;
}

IFACEMETHODIMP ClassFactory::LockServer(BOOL fLock) {
    if(fLock)
        InterlockedIncrement(&g_cDllRef);
    else
        InterlockedDecrement(&g_cDllRef);
    return S_OK;
}
