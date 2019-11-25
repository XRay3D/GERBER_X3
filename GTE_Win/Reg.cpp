#include "Reg.h"
#include <strsafe.h>

#pragma region Registry Helper Functions

//
//   FUNCTION: SetHKCRRegistryKeyAndValue
//
//   PURPOSE: The function creates a HKCR registry key and sets the specified
//   registry value.
//
//   PARAMETERS:
//   * pszSubKey - specifies the registry key under HKCR. If the key does not
//     exist, the function will create the registry key.
//   * pszValueName - specifies the registry value to be set. If pszValueName
//     is NULL, the function will set the default value.
//   * pszData - specifies the string data of the registry value.
//
//   RETURN VALUE:
//   If the function succeeds, it returns S_OK. Otherwise, it returns an
//   HRESULT error code.
//
HRESULT SetHKCRRegistryKeyAndValue(PCWSTR pszSubKey, PCWSTR pszValueName,
    PCWSTR pszData)
{
    HRESULT hr;
    HKEY hKey = nullptr;

    // Creates the specified registry key. If the key already exists, the
    // function opens it.
    hr = HRESULT_FROM_WIN32(static_cast<unsigned>(RegCreateKeyEx(HKEY_CLASSES_ROOT, pszSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr)));

    if (SUCCEEDED(hr)) {
        if (pszData != nullptr) {
            // Set the specified value of the key.
            DWORD cbData = static_cast<unsigned>(lstrlen(pszData)) * sizeof(*pszData);
            hr = HRESULT_FROM_WIN32(static_cast<unsigned>(RegSetValueEx(hKey, pszValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(pszData), cbData)));
        }

        RegCloseKey(hKey);
    }

    return hr;
}

//
//   FUNCTION: GetHKCRRegistryKeyAndValue
//
//   PURPOSE: The function opens a HKCR registry key and gets the data for the
//   specified registry value name.
//
//   PARAMETERS:
//   * pszSubKey - specifies the registry key under HKCR. If the key does not
//     exist, the function returns an error.
//   * pszValueName - specifies the registry value to be retrieved. If
//     pszValueName is NULL, the function will get the default value.
//   * pszData - a pointer to a buffer that receives the value's string data.
//   * cbData - specifies the size of the buffer in bytes.
//
//   RETURN VALUE:
//   If the function succeeds, it returns S_OK. Otherwise, it returns an
//   HRESULT error code. For example, if the specified registry key does not
//   exist or the data for the specified value name was not set, the function
//   returns COR_E_FILENOTFOUND (0x80070002).
//
HRESULT GetHKCRRegistryKeyAndValue(PCWSTR pszSubKey, PCWSTR pszValueName, PWSTR pszData, DWORD cbData)
{
    HRESULT hr;
    HKEY hKey = nullptr;

    // Try to open the specified registry key.
    hr = HRESULT_FROM_WIN32(static_cast<unsigned>(RegOpenKeyEx(HKEY_CLASSES_ROOT, pszSubKey, 0, KEY_READ, &hKey)));
    ;

    if (SUCCEEDED(hr)) {
        // Get the data for the specified value name.
        hr = HRESULT_FROM_WIN32(static_cast<unsigned>(RegQueryValueEx(hKey, pszValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(pszData), &cbData)));
        ;
        RegCloseKey(hKey);
    }

    return hr;
}

#pragma endregion

//
//   FUNCTION: RegisterInprocServer
//
//   PURPOSE: Register the in-process component in the registry.
//
//   PARAMETERS:
//   * pszModule - Path of the module that contains the component
//   * clsid - Class ID of the component
//   * pszFriendlyName - Friendly name
//   * pszThreadModel - Threading model
//
//   NOTE: The function creates the HKCR\CLSID\{<CLSID>} key in the registry.
//
//   HKCR
//   {
//      NoRemove CLSID
//      {
//          ForceRemove {<CLSID>} = s '<Friendly Name>'
//          {
//              InprocServer32 = s '%MODULE%'
//              {
//                  val ThreadingModel = s '<Thread Model>'
//              }
//          }
//      }
//   }
//
HRESULT RegisterInprocServer(PCWSTR pszModule, const CLSID& clsid,
    PCWSTR pszFriendlyName, PCWSTR pszThreadModel)
{
    if (pszModule == nullptr || pszThreadModel == nullptr) {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    // Create the HKCR\CLSID\{<CLSID>} key.
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    if (SUCCEEDED(hr)) {
        hr = SetHKCRRegistryKeyAndValue(szSubkey, nullptr, pszFriendlyName);

        // Create the HKCR\CLSID\{<CLSID>}\InprocServer32 key.
        if (SUCCEEDED(hr)) {
            hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
                L"CLSID\\%s\\InprocServer32", szCLSID);
            if (SUCCEEDED(hr)) {
                // Set the default value of the InprocServer32 key to the
                // path of the COM module.
                hr = SetHKCRRegistryKeyAndValue(szSubkey, nullptr, pszModule);
                if (SUCCEEDED(hr)) {
                    // Set the threading model of the component.
                    hr = SetHKCRRegistryKeyAndValue(szSubkey, L"ThreadingModel", pszThreadModel);
                }
            }
        }
    }

    return hr;
}

//
//   FUNCTION: UnregisterInprocServer
//
//   PURPOSE: Unegister the in-process component in the registry.
//
//   PARAMETERS:
//   * clsid - Class ID of the component
//
//   NOTE: The function deletes the HKCR\CLSID\{<CLSID>} key in the registry.
//
HRESULT UnregisterInprocServer(const CLSID& clsid)
{
    HRESULT hr = S_OK;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    // Delete the HKCR\CLSID\{<CLSID>} key.
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    if (SUCCEEDED(hr)) {
        hr = HRESULT_FROM_WIN32(static_cast<unsigned>(RegDeleteTree(HKEY_CLASSES_ROOT, szSubkey)));
        ;
    }

    return hr;
}

//
//   FUNCTION: RegisterShellExtThumbnailHandler
//
//   PURPOSE: Register the thumbnail handler.
//
//   PARAMETERS:
//   * pszFileType - The file type that the thumbnail handler is associated
//     with. For example, '*' means all file types; '.txt' means all .txt
//     files. The parameter must not be NULL.
//   * clsid - Class ID of the component
//
//   NOTE: The function creates the following key in the registry.
//
//   HKCR
//   {
//      NoRemove <File Type>
//      {
//          NoRemove shellex
//          {
//              {e357fccd-a995-4576-b01f-234630154e96} = s '{<CLSID>}'
//          }
//      }
//   }
//
HRESULT RegisterShellExtThumbnailHandler(PCWSTR pszFileType, const CLSID& clsid)
{
    if (pszFileType == nullptr) {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    // If pszFileType starts with '.', try to read the default value of the
    // HKCR\<File Type> key which contains the ProgID to which the file type
    // is linked.
    if (*pszFileType == L'.') {
        wchar_t szDefaultVal[260];
        hr = GetHKCRRegistryKeyAndValue(pszFileType, nullptr, szDefaultVal, sizeof(szDefaultVal));

        // If the key exists and its default value is not empty, use the
        // ProgID as the file type.
        if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0') {
            pszFileType = szDefaultVal;
        }
    }

    // Create the registry key
    // HKCR\<File Type>\shellex\{e357fccd-a995-4576-b01f-234630154e96}
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"%s\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}", pszFileType);
    if (SUCCEEDED(hr)) {
        // Set the default value of the key.
        hr = SetHKCRRegistryKeyAndValue(szSubkey, nullptr, szCLSID);
    }

    return hr;
}

//
//   FUNCTION: UnregisterShellExtThumbnailHandler
//
//   PURPOSE: Unregister the thumbnail handler.
//
//   PARAMETERS:
//   * pszFileType - The file type that the thumbnail handler is associated
//     with. For example, '*' means all file types; '.txt' means all .txt
//     files. The parameter must not be NULL.
//
//   NOTE: The function removes the registry key
//   HKCR\<File Type>\shellex\{e357fccd-a995-4576-b01f-234630154e96}.
//
HRESULT UnregisterShellExtThumbnailHandler(PCWSTR pszFileType)
{
    if (pszFileType == nullptr) {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szSubkey[MAX_PATH];

    // If pszFileType starts with '.', try to read the default value of the
    // HKCR\<File Type> key which contains the ProgID to which the file type
    // is linked.
    if (*pszFileType == L'.') {
        wchar_t szDefaultVal[260];
        hr = GetHKCRRegistryKeyAndValue(pszFileType, nullptr, szDefaultVal, sizeof(szDefaultVal));

        // If the key exists and its default value is not empty, use the
        // ProgID as the file type.
        if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0') {
            pszFileType = szDefaultVal;
        }
    }

    // Remove the registry key:
    // HKCR\<File Type>\shellex\{e357fccd-a995-4576-b01f-234630154e96}
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
        L"%s\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}", pszFileType);
    if (SUCCEEDED(hr)) {
        hr = HRESULT_FROM_WIN32(static_cast<unsigned>(RegDeleteTree(HKEY_CLASSES_ROOT, szSubkey)));
        ;
    }

    return hr;
}
