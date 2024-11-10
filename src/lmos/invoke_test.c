/**
 * @file lmos.c
 * @brief This program demonstrates how to instantiate an ActiveX control, obtain its IDispatch pointer, and call its
 * methods, all using plain C.
 */

#include <ole2.h>
#include <oleauto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


/* ░█▀▀░▀█▀░█▀▄░█░█░█▀▀░▀█▀░█▀▀ */
/* ░▀▀█░░█░░█▀▄░█░█░█░░░░█░░▀▀█ */
/* ░▀▀▀░░▀░░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀ */

struct activexctrl
{
	HRESULT	    hr;	 // Last result
	ILockBytes* plkbyt;
	IStorage*   pStorage;
	IOleObject* pActiveXControl;
	IDispatch*  pDispatch;
	ITypeInfo*  pTypeInfo;
	ITypeLib*   pTypeLib;
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
StringToBSTR( const char* string )
{  // Allocates a BSTR from a char string (must be freed with SysFreeString)

	char* wide_string = calloc( 1, ( strlen( string ) + 1 ) * 2 );
	mbstowcs( (wchar_t*)wide_string, string, strlen( string ) );
	BSTR bstr_string = SysAllocString( (wchar_t*)wide_string );
	free( wide_string );
	return ( bstr_string );
}

HRESULT
GetDispId( IDispatch *pDispatch, const wchar_t *method, DISPID *dispid )
{
    // Obtain a DISPID for a given method name
	BSTR	bstrMethodName = SysAllocString( method );
	HRESULT hr = pDispatch->lpVtbl->GetIDsOfNames(
		pDispatch, &IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, dispid );
	SysFreeString( bstrMethodName );
    return hr;
}

/* ░▀█▀░█▀▀░█▀▀░▀█▀░░░█▄█░█▀▀░▀█▀░█░█░█▀█░█▀▄░█▀▀ */
/* ░░█░░█▀▀░▀▀█░░█░░░░█░█░█▀▀░░█░░█▀█░█░█░█░█░▀▀█ */
/* ░░▀░░▀▀▀░▀▀▀░░▀░░░░▀░▀░▀▀▀░░▀░░▀░▀░▀▀▀░▀▀░░▀▀▀ */

int
ShowAboutBox( IDispatch* pDispatch )
{
	// Display the About Box

	// Obtain the DISPID for the "AboutBox" method
	/* BSTR   bstrMethodName = SysAllocString( L"ClearLayout" ); */
	/* BSTR   bstrMethodName = SysAllocString( L"CancelJob" ); */
	BSTR	bstrMethodName = SysAllocString( L"AboutBox" );
	DISPID	dispidAboutBoxMethod;
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
LoadFile( IDispatch* pDispatch )
{
	/* We must force a clear here, since Lmos crashes if a previous layout
	 * exists */
	/* lmos_actx->CancelJob(); */
	/* lmos_actx->ClearLayout(); */
	/**/
	/* bool result = lmos_actx->LoadXML( xml ); */
	/* lmos_actx->ShowMarkingArea(); */

	// Obtain the DISPID for the "FileName2" method
	BSTR	bstrMethodName = SysAllocString( L"FileName2" );
	DISPID	dispidAboutBoxMethod;
	HRESULT hr = pDispatch->lpVtbl->GetIDsOfNames(
		pDispatch, &IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispidAboutBoxMethod );
	SysFreeString( bstrMethodName );
	if ( !CheckHR( hr, TEXT( "Failed to get DISPID for FileName2 method" ) ) ) {
		return 1;
	}

	// Prepare and invoke the "FileName2" method
	DISPPARAMS params = { NULL, NULL, 0, 0 };
	VARIANT	   retVal;
	VariantInit( &retVal );
	EXCEPINFO excepinfo;
	memset( &excepinfo, 0, sizeof( excepinfo ) );
	REFIID riid   = &IID_NULL;
	LCID   lcid   = LOCALE_USER_DEFAULT;
	WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

	// Call the method
	BSTR	   bstrFileName = SysAllocString( L"dummy" );
	VARIANTARG arg[1];
	arg[0].vt      = VT_BSTR;
	arg[0].bstrVal = bstrFileName;
	params.rgvarg  = arg;
	params.cArgs   = 1;

	hr = pDispatch->lpVtbl->Invoke(
		pDispatch, dispidAboutBoxMethod, riid, lcid, wFlags, &params, &retVal, &excepinfo, NULL );

	if ( !CheckHR( hr, TEXT( "Failed to invoke FileName2 method" ) ) ) {
		DWORD dwError = HRESULT_FROM_WIN32( GetLastError() );
		fprintf( stderr,
			 TEXT( "Failed to invoke FileName2 method: HRESULT=0x%08lX, Win32Error=0x%08lX\n" ),
			 hr,
			 dwError );

		return 1;
	}

	// Release resources and clean up
	SysFreeString( bstrFileName );
	VariantClear( &retVal );

	return 0;
}

int
GetVlmVersion(IDispatch *pDispatch)
{

    printf("getting marking files path\n");
    DISPID dispid;
    GetDispId( pDispatch, L"GetMarkingFilesPath", &dispid );

    // Obtain the DISPID for the "VLMVersion" method
    /* BSTR   bstrMethodName = SysAllocString( L"VLMVersion" ); */
    /* DISPID dispidAboutBoxMethod; */
    /* hr = pDispatch->lpVtbl->GetIDsOfNames( */
    /*     pDispatch, &IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispidAboutBoxMethod ); */
    /* SysFreeString( bstrMethodName ); */
    /* if ( !CheckHR( hr, TEXT( "Failed to get DISPID for VLMVersion method" ) ) ) { */
    /*     return 1; */
    /* } */

    // Prepare and invoke the "VLMVersion" method
    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT	   retVal;
    VariantInit( &retVal );
    EXCEPINFO excepinfo;
    memset( &excepinfo, 0, sizeof( excepinfo ) );
    REFIID riid   = &IID_NULL;
    LCID   lcid   = LOCALE_USER_DEFAULT;
    WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

    // Call the method
    HRESULT hr = pDispatch->lpVtbl->Invoke(
        pDispatch, dispid, riid, lcid, wFlags, &params, &retVal, &excepinfo, NULL );

    if ( !CheckHR( hr, TEXT( "Failed to invoke GetMarkingFilesPath method" ) ) ) {
        DWORD dwError = HRESULT_FROM_WIN32( GetLastError() );
        fprintf( stderr,
             TEXT( "Failed to invoke VLMVersion method: HRESULT=0x%08lX, Win32Error=0x%08lX\n" ),
             hr,
             dwError );
        return 1;
    }

    if ( retVal.vt == VT_BSTR ) {
        wprintf( L"VLM Version: %s\n", retVal.bstrVal );
    } else {
        fprintf( stderr, TEXT( "Failed to invoke VLMVersion method: wrong return type\n" ) );
        return 1;
    }

    VariantClear( &retVal );
}

int
InitializeActiveXControl(struct activexctrl *control)
{
	// Initialize COM libraries
	control->hr= CoInitialize( NULL );
	if ( !CheckHR( control->hr, TEXT( "CoInitialize failed" ) ) ) {
		return 1;
	}

	control->hr= OleInitialize( NULL );
	if ( !CheckHR( control->hr, TEXT( "OleInitialize failed" ) ) ) {
		return 1;
	}

	// Allocate storage for OleCreate
	control->plkbyt = NULL;
	control->hr		   = CreateILockBytesOnHGlobal( NULL, TRUE, &control->plkbyt );
	if ( !CheckHR( control->hr, TEXT( "CreateILockBytesOnHGlobal failed" ) ) ) {
		return 1;
	}

	control->pStorage = NULL;
	control->hr= StgCreateDocfileOnILockBytes( control->plkbyt, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &control->pStorage );
	if ( !CheckHR( control->hr, TEXT( "StgCreateDocfileOnILockBytes failed" ) ) ) {
		return 1;
	}

    /* return 0; */
	// Instantiate the ActiveX control
	control->pActiveXControl = NULL;
	const CLSID clsid = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };
	control->hr= OleCreate( &clsid, &IID_IOleObject, OLERENDER_DRAW, NULL, NULL, control->pStorage, (LPVOID*)&control->pActiveXControl );
	if ( !CheckHR( control->hr, TEXT( "OleCreate failed" ) ) ) {
		return 1;
	}

	// Get the dispatch pointer
	control->hr= control->pActiveXControl->lpVtbl->QueryInterface( control->pActiveXControl, &IID_IDispatch, (LPVOID*)&control->pDispatch );
	if ( !CheckHR( control->hr, TEXT( "QueryInterface failed for IID_IDispatch" ) ) ) {
		return 1;
	}

	// Get the ITypeInfo interface for the IDispatch object
	control->hr= control->pDispatch->lpVtbl->GetTypeInfo( control->pDispatch, 0, LOCALE_USER_DEFAULT, &control->pTypeInfo );
	if ( !CheckHR( control->hr, TEXT( "Failed to get type information" ) ) ) {
		return 1;
	}

    /* return 0; */
	// Obtain the type library of the ActiveX control
	UINT	  index;
	control->hr= control->pTypeInfo->lpVtbl->GetContainingTypeLib( control->pTypeInfo, &control->pTypeLib, &index );
	control->pTypeInfo->lpVtbl->Release( control->pTypeInfo );
	if ( !CheckHR( control->hr, TEXT( "Failed to obtain type library" ) ) ) {
		return 1;
	}

    return 0;

}

void
ReleaseActiveXControl(struct activexctrl *control)
{
	// Release resources
	control->pTypeLib->lpVtbl->Release( control->pTypeLib );
	control->pDispatch->lpVtbl->Release( control->pDispatch );
	control->pActiveXControl->lpVtbl->Release( control->pActiveXControl );
	control->pStorage->lpVtbl->Release( control->pStorage );
	control->plkbyt->lpVtbl->Release( control->plkbyt );
	CoUninitialize();
}

int
main()
{
    struct activexctrl lmos;

    printf("Initializing control...\n");
    int result = InitializeActiveXControl(&lmos);
    if(result) return -1;
    
    GetVlmVersion(lmos.pDispatch);
    /* ShowAboutBox(lmos.pDispatch); */

	// Show window
	HRESULT hr = lmos.pActiveXControl->lpVtbl->DoVerb(
		lmos.pActiveXControl, OLEIVERB_SHOW, NULL, (IOleClientSite*)lmos.pActiveXControl, 0, NULL, NULL );
	if ( !CheckHR( hr, TEXT( "Failed to call DoVerb" ) ) ) {
		return 1;
	}

    // Run tests

    // Clean up
    printf("Releasing control...\n");
    ReleaseActiveXControl(&lmos);

	return 0;
}


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
