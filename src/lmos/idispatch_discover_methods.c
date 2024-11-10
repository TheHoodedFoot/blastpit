// Can you write a simple C++ program that gives an example of how to use IDispatch to discover the available methods of an ActiveX control?

// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/automat/accessing-members-through-idispatch

#include <stdio.h>
#include <windows.h>
// #include <comutil.h>

int main() {
    // Initialize COM library
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "CoInitialize failed: 0x%08X\n", hr);
        return 1;
    }

    // Replace this with the GUID of your ActiveX control
    CLSID clsidActiveX = { 0x18213698, 0xA9C9, 0x11D1, 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 };

    // Create an instance of the ActiveX control
    IDispatch* pDispatch = NULL;
    hr = CoCreateInstance(&clsidActiveX, NULL, CLSCTX_INPROC_SERVER, &IID_IDispatch, (void**)&pDispatch);
    if (FAILED(hr)) {
        fprintf(stderr, "CoCreateInstance failed: 0x%08X\n", hr);
        CoUninitialize();
        return 1;
    }

    // Get the type information for the ActiveX control
    ITypeInfo* pTypeInfo = NULL;
    hr = pDispatch->lpVtbl->GetTypeInfo(pDispatch, 0, GetUserDefaultLCID(), &pTypeInfo);
    if (FAILED(hr)) {
        fprintf(stderr, "GetTypeInfo failed: 0x%08X\n", hr);
        pDispatch->lpVtbl->Release(pDispatch);
        CoUninitialize();
        return 1;
    }

    // Get the containing type library
    ITypeLib* pTypeLib = NULL;
    hr = pTypeInfo->lpVtbl->GetContainingTypeLib(pTypeInfo, &pTypeLib, NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "GetContainingTypeLib failed: 0x%08X\n", hr);
        pTypeInfo->lpVtbl->Release(pTypeInfo);
        pDispatch->lpVtbl->Release(pDispatch);
        CoUninitialize();
        return 1;
    }

    printf("Available methods:\n");

    // Enumerate and display the available methods
    printf("pTypeLib->lpVtbl->GetTypeInfoCount(pTypeLib): %d\n", pTypeLib->lpVtbl->GetTypeInfoCount(pTypeLib));
    for (UINT i = 0; i < pTypeLib->lpVtbl->GetTypeInfoCount(pTypeLib); i++) {
        ITypeInfo* pTypeInfoInLib;
        hr = pTypeLib->lpVtbl->GetTypeInfo(pTypeLib, i, &pTypeInfoInLib);
        if (SUCCEEDED(hr)) {
            TYPEATTR* pTypeAttr;
            hr = pTypeInfoInLib->lpVtbl->GetTypeAttr(pTypeInfoInLib, &pTypeAttr);
            if (SUCCEEDED(hr)) {
                for (UINT j = 0; j < pTypeAttr->cFuncs; j++) {
                    FUNCDESC* pFuncDesc;
                    hr = pTypeInfoInLib->lpVtbl->GetFuncDesc(pTypeInfoInLib, j, &pFuncDesc);
                    if (SUCCEEDED(hr)) {
                        // Check if the method can be called using IDispatch
                        if (pFuncDesc->invkind & INVOKE_FUNC) {
                            // This is a method that can be called using IDispatch
                            // You can inspect other attributes of the method here
                            BSTR funcName;
                            hr = pTypeInfoInLib->lpVtbl->GetDocumentation(pTypeInfoInLib, pFuncDesc->memid, &funcName, NULL, NULL, NULL);
                            if (SUCCEEDED(hr)) {
                                printf("IDispatch Method(%d, %d): %ls\n", i, j, funcName);
                                SysFreeString(funcName);
                            }
                        }
                        pTypeInfoInLib->lpVtbl->ReleaseFuncDesc(pTypeInfoInLib, pFuncDesc);
                    }
                }
                pTypeInfoInLib->lpVtbl->ReleaseTypeAttr(pTypeInfoInLib, pTypeAttr);
            }
            pTypeInfoInLib->lpVtbl->Release(pTypeInfoInLib);
        }
    }

    // Release COM objects
    pTypeLib->lpVtbl->Release(pTypeLib);
    pTypeInfo->lpVtbl->Release(pTypeInfo);
    pDispatch->lpVtbl->Release(pDispatch);
    CoUninitialize();

    return 0;
}

// vim: formatoptions-=cro
