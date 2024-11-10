/**
 * @file lmos.c
 * @brief This program demonstrates how to instantiate an ActiveX control, obtain its IDispatch pointer, and call its methods, all using plain C.
 */

#include <ole2.h>
#include <stdio.h>
#include <windows.h>


/* ░█▀▀░▀█▀░█▀▄░█░█░█▀▀░▀█▀░█▀▀ */
/* ░▀▀█░░█░░█▀▄░█░█░█░░░░█░░▀▀█ */
/* ░▀▀▀░░▀░░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀ */

struct activexctrl {
    HRESULT hr; // Last result
    ILockBytes* plkbyt;
    IStorage* pStorage;
    IOleObject* pActiveXControl;
    IDispatch* pDispatch;
    ITypeInfo* pTypeInfo;
    ITypeLib* pTypeLib;
};


// ░█▀▄░█▀▀░█▀▄░█░█░█▀▀░░░█░█░█▀▀░█░░░█▀█░█▀▀░█▀▄░█▀▀
// ░█░█░█▀▀░█▀▄░█░█░█░█░░░█▀█░█▀▀░█░░░█▀▀░█▀▀░█▀▄░▀▀█
// ░▀▀░░▀▀▀░▀▀░░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀▀▀░▀░░░▀▀▀░▀░▀░▀▀▀

/* Check if the given HRESULT indicates success or failure, print error message on failure */
BOOL
CheckHR( HRESULT hr, LPCTSTR szMessage )
{
    if ( FAILED( hr ) ) {
		MessageBox( 0, szMessage, "Error", MB_OK | MB_ICONEXCLAMATION );
        fprintf( stderr, TEXT( "%s: 0x%08lX\n" ), szMessage, hr );
        return FALSE;
    }
    return TRUE;
}


// ░█░█░▀█▀░▀█▀░█░░░▀█▀░▀█▀░▀█▀░█▀▀░█▀▀
// ░█░█░░█░░░█░░█░░░░█░░░█░░░█░░█▀▀░▀▀█
// ░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀

BSTR
StringToBSTR( const char *string )
{ // Allocates a BSTR from a char string (must be freed with SysFreeString

	char* wide_string = calloc( 1, ( strlen( string ) + 1 ) * 2 );
	mbstowcs( (wchar_t*)wide_string, string, strlen( string ) );
	BSTR bstr_string = SysAllocString( (wchar_t*)wide_string );
	free( wide_string );
	return (bstr_string);
}

    int ShowAboutBox(IDispatch *pDispatch)
    {
    // Display the About Box

    // Obtain the DISPID for the "AboutBox" method
    /* BSTR   bstrMethodName = SysAllocString( L"ClearLayout" ); */
    /* BSTR   bstrMethodName = SysAllocString( L"CancelJob" ); */
    BSTR   bstrMethodName = SysAllocString( L"AboutBox" );
    DISPID dispidAboutBoxMethod;
    HRESULT hr = pDispatch->lpVtbl->GetIDsOfNames(
        pDispatch, &IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispidAboutBoxMethod );
    SysFreeString( bstrMethodName );
    if ( !CheckHR( hr, TEXT( "Failed to get DISPID for AboutBox method" ) ) ) {
        return 1;
    }

    // Prepare and invoke the "AboutBox" method
    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT	   retVal;
    VariantInit( &retVal );
    EXCEPINFO excepinfo;
    memset( &excepinfo, 0, sizeof( excepinfo ) );
    REFIID riid   = &IID_NULL;
    LCID   lcid   = LOCALE_USER_DEFAULT;
    WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

    // Call the method
    hr = pDispatch->lpVtbl->Invoke(
        pDispatch, dispidAboutBoxMethod, riid, lcid, wFlags, &params, &retVal, &excepinfo, NULL );

    if ( !CheckHR( hr, TEXT( "Failed to invoke AboutBox method" ) ) ) {
        DWORD dwError = HRESULT_FROM_WIN32( GetLastError() );
        fprintf( stderr,
                TEXT( "Failed to invoke AboutBox method: HRESULT=0x%08lX, Win32Error=0x%08lX\n" ),
                hr,
                dwError );
        return 1;
    }

    // Release resources and clean up
    VariantClear( &retVal );

    return 0;
    }

int
main()
{
    // Initialize COM libraries
    HRESULT hr = CoInitialize( NULL );
    if ( !CheckHR( hr, TEXT( "CoInitialize failed" ) ) ) {
        return 1;
    }

    hr = OleInitialize( NULL );
    if ( !CheckHR( hr, TEXT( "OleInitialize failed" ) ) ) {
        return 1;
    }

    // Allocate storage for OleCreate
    ILockBytes* plkbyt = NULL;
    hr		   = CreateILockBytesOnHGlobal( NULL, TRUE, &plkbyt );

    IStorage* pStorage = NULL;
    hr = StgCreateDocfileOnILockBytes( plkbyt, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pStorage );

    // Instantiate the ActiveX control
    IOleObject* pActiveXControl = NULL;
    const CLSID clsid = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };
    hr = OleCreate( &clsid, &IID_IOleObject, OLERENDER_DRAW, NULL, NULL, pStorage, (LPVOID*)&pActiveXControl );
    if ( !CheckHR( hr, TEXT( "OleCreate failed" ) ) ) {
        return 1;
    }

    // Get the dispatch pointer
    IDispatch* pDispatch;
    hr = pActiveXControl->lpVtbl->QueryInterface( pActiveXControl, &IID_IDispatch, (LPVOID*)&pDispatch );
    if ( !CheckHR( hr, TEXT( "QueryInterface failed for IID_IDispatch" ) ) ) {
        return 1;
    }

    // Get the ITypeInfo interface for the IDispatch object
    ITypeInfo* pTypeInfo;
    hr = pDispatch->lpVtbl->GetTypeInfo( pDispatch, 0, LOCALE_USER_DEFAULT, &pTypeInfo );
    if ( !CheckHR( hr, TEXT( "Failed to get type information" ) ) ) {
        return 1;
    }

    // Obtain the type library of the ActiveX control
    ITypeLib* pTypeLib;
    UINT	  index;
    hr = pTypeInfo->lpVtbl->GetContainingTypeLib( pTypeInfo, &pTypeLib, &index );
    pTypeInfo->lpVtbl->Release( pTypeInfo );
    if ( !CheckHR( hr, TEXT( "Failed to obtain type library" ) ) ) {
        return 1;
    }

    // Show window
    hr = pActiveXControl->lpVtbl->DoVerb(pActiveXControl, OLEIVERB_SHOW, NULL, (IOleClientSite*)pActiveXControl, 0, NULL, NULL);
    if ( !CheckHR( hr, TEXT( "Failed to call DoVerb" ) ) ) {
        return 1;
    }

    // Release resources
    pTypeLib->lpVtbl->Release( pTypeLib );
    pDispatch->lpVtbl->Release( pDispatch );
    pActiveXControl->lpVtbl->Release( pActiveXControl );
    pStorage->lpVtbl->Release( pStorage );
    plkbyt->lpVtbl->Release( plkbyt );
    CoUninitialize();

    return 0;
}


/* We must force a clear here, since Lmos crashes if a previous layout
     * exists */
/* lmos_actx->CancelJob(); */
/* lmos_actx->ClearLayout(); */
/**/
/* bool result = lmos_actx->LoadXML( xml ); */
/* lmos_actx->ShowMarkingArea(); */

/*

These are the functions exposed by LMOS


Required
--------

InitMachine()
Save(filename)
FileName2(filename) (load VLM file)
LoadXML(xml)
GetGlobalQPSetNames()
RemoveGlobalQPSet(qpnameslist)
CancelJob()
ClearLayout()
LoadXML(xml)
ImportXMLFile2(file)
StartPosHelp(object)
StopPosHelp()
TermMachine()
StartMarking()
LoadJob()
StopMarking()
SaveGlobalQPSets
Layers()
layer.SetHeightZAxis
AddGlobalQPSet
SetDimension
SetQualityParam
GetMONames
GetPosValues
GetDimension


Optional
--------
show()
hide()
grab()
ShowMarkingArea()
ActivateZoomWindow(bool)
item.SetVisible
item.SetExportable
axis.MoveAxes();
axis.GetPos
axis.ReferenceAxes
axis.NewReference
Axis()
ReadByte
ReadIOBit
SetMOLayer
WriteByte
WriteIOBit
ShowZoomWindow
ShowMarkingAreaZoom
SetLaserable
RedrawLayout
SetSuppressAutoRedraw
*/



// Windows Registry Editor Version 5.00
//
// [HKEY_CLASSES_ROOT\LMOSACTX.LMOSActXCtrl.1\CLSID]
// @="{18213698-A9C9-11D1-A220-0060973058F6}"
//
//
