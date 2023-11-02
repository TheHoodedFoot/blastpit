#include <windows.h>
#include <cstdio>
#include <oaidl.h>

void SetVisible(IDispatch* pApp, LCID lcid)
{
 VARIANT         vArgArray[1];
 DISPPARAMS      DispParams = { NULL, NULL, 0, 0 };
 // DISPID          dispidNamed;
 VARIANT         vResult;

 VariantInit(&vArgArray[0]);
 // vArgArray[0].vt                = VT_BOOL;
 // vArgArray[0].boolVal           = TRUE;
 // dispidNamed                    = DISPID_COLLECT;
 VariantInit(&vResult);
 EXCEPINFO excep;
 HRESULT hr = pApp->Invoke(0xfffffdd8,IID_NULL,lcid,DISPATCH_METHOD,&DispParams,&vResult,&excep,NULL);
 fprintf(stderr, "invoke: %0x\n", hr);
 fprintf(stderr, "excepinfo wCode:%0x, scode: %0x\n", excep.wCode, excep.scode);
}

int main()
{
 const CLSID      CLSID_LMOS = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };
 // const CLSID      CLSID_IE_Application = {0x0002DF01,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
 // const IID        IID_IEApplication    = {0x0002DF05,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
 IDispatch*       pApp                 = NULL;
 LCID             lcid;
 HRESULT          hr;

 hr=CoInitialize(NULL);
 if(SUCCEEDED(hr))
 {
    fprintf(stderr, "CoInitialize() Succeeded!\n");
    hr = CoCreateInstance(CLSID_LMOS, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **) &pApp);
    if(SUCCEEDED(hr))
    {
       fprintf(stderr, "pApp = 0x%p\n",pApp);
       lcid=GetUserDefaultLCID();
       SetVisible(pApp,lcid);
       // getchar();
       pApp->Release();
    }
    else
       printf("CoCreateInstance() Failed!\n");
    CoUninitialize();
 }
 else
    printf("CoInitialize() Failed!\n");
 getchar();

 return 0;
}
