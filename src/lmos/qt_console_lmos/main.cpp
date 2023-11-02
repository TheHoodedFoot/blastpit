#include <iostream>
#include <ole2.h>

using namespace std;

int main(int argc, char* argv[])
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "CoInitialize failed: 0x%08X\n", hr);
        return 1;
    }

    // Instantiate the ActiveX control.
    IUnknown* pUnknown = 0;
    IUnknown** ppUnkn = &pUnknown;

    const CLSID  CLSID_LMOS = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };
    hr = CoCreateInstance(CLSID_LMOS, 0, CLSCTX_SERVER, IID_IUnknown, (void**)ppUnkn);

    if (FAILED(hr))
    {
        printf("Failed to instantiate ActiveX control: %08x\n", hr);
        return 1;
    }

    // Get the disaptch
    IDispatch *pDispatch;
    hr = pUnknown->QueryInterface(IID_IDispatch, (void**)&pDispatch);
    if (FAILED(hr))
    {
        printf("QueryInterface failed for IID_IDispatch: %08x\n", hr);
        return 1;
    }


    // Get the ITypeInfo interface for the IDispatch object.
    ITypeInfo* pTypeInfo;
    hr = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
    if (FAILED(hr))
    {
        printf("Failed to get type information: %08x\n", hr);
        return 1;
    }

    // Obtain the type library of the ActiveX control.
    ITypeLib* pTypeLib;
    UINT index;
    hr = pTypeInfo->GetContainingTypeLib(&pTypeLib, &index);
    pTypeInfo->Release();
    if (FAILED(hr))
    {
        printf("Failed to obtain type library: %08x\n", hr);
        return 1;
    }
   // Step 6: Obtain the DISPID (Dispatch Identifier) for the "AboutBox" method.
    BSTR bstrMethodName = SysAllocString(L"AboutBox");
    DISPID dispidMethod;
   hr = pDispatch->GetIDsOfNames(IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispidMethod);
   SysFreeString(bstrMethodName); // Release the allocated BSTR
   if (FAILED(hr))
   {
       printf("Failed to get DISPID for method: %08x\n", hr);
       return 1;
   }

   // Step 7: Prepare and invoke the "AboutBox" method.
   DISPPARAMS params = { NULL, NULL, 0, 0 };
   VARIANT ret;
   VariantInit(&ret);
   UINT argerr = 0;
   EXCEPINFO excepinfo;
   memset(&excepinfo, 0, sizeof(excepinfo));
   REFIID myid = IID_NULL;
   LCID mylcid = LOCALE_USER_DEFAULT;
   WORD wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

   // call the method
   hr = pDispatch->Invoke(dispidMethod, myid, mylcid, wFlags, &params, &ret, &excepinfo, &argerr);

   if (FAILED(hr))
   {
    DWORD dwError = HRESULT_FROM_WIN32(GetLastError());
    printf("Failed to invoke method: HRESULT=0x%08X, Win32Error=0x%08X\n", hr, dwError);
    return 1;
   }

   // Step 8: Release resources and clean up.
   VariantClear(&ret);
   pTypeLib->Release();
   pUnknown->Release();
   CoUninitialize();

   return 0;
}

// Error codes in Winerror.h
