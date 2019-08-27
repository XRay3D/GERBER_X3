#pragma once

#include <Unknwn.h> // For IClassFactory
#include <qt_windows.h>

class ClassFactory : public IClassFactory {
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG)
    AddRef() override;
    IFACEMETHODIMP_(ULONG)
    Release() override;

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override;
    IFACEMETHODIMP LockServer(BOOL fLock) override;

    ClassFactory();

protected:
    ~ClassFactory();

private:
    unsigned long m_cRef;
};
