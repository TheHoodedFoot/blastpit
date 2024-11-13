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


// Structs
struct activexctrl
{
	HRESULT	    hr;	 // Last result
	ILockBytes* plkbyt;
	IStorage*   pStorage;
	IOleObject* pActiveXControl;
	IDispatch*  pDispatch;
	ITypeInfo*  pTypeInfo;
	ITypeLib*   pTypeLib;
	HWND	    hwnd;  // Handle to the control's window
};


// Debug helper functions
BOOL
CheckHR( HRESULT hr, LPCTSTR szMessage )
{
	// Check if the given HRESULT indicates success or failure, print error message on failure
	if ( FAILED( hr ) ) {
		MessageBox( 0, szMessage, "Error", MB_OK | MB_ICONEXCLAMATION );
		fprintf( stderr, TEXT( "%s: 0x%08lX\n" ), szMessage, hr );
		return FALSE;
	}
	return TRUE;
}


// Utility functions
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
GetDispId( IDispatch* pDispatch, const wchar_t* method, DISPID* dispid )
{
	// Obtain a DISPID for a given method name
	BSTR	bstrMethodName = SysAllocString( method );
	HRESULT hr	       = pDispatch->lpVtbl->GetIDsOfNames(
		    pDispatch, &IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, dispid );
	SysFreeString( bstrMethodName );
	return hr;
}


// Custom functions to call the ActiveX control's methods by name
int
StringActiveXHr( IDispatch* pDispatch, const unsigned short* methodname, VARIANT* retval )
{
	/** Calls an ActiveX control's method by name
	 * The method takes no arguments and returns a string
	 *
	 * @param retval Pointer to VARIANT to hold result
	 * @return HRESULT of method call
	 */

	printf( "getting marking files path\n" );
	DISPID dispid;
	GetDispId( pDispatch, L"GetMarkingFilesPath", &dispid );

	DISPPARAMS params = { NULL, NULL, 0, 0 };
	VariantInit( retval );
	EXCEPINFO excepinfo;
	memset( &excepinfo, 0, sizeof( excepinfo ) );
	REFIID riid   = &IID_NULL;
	LCID   lcid   = LOCALE_USER_DEFAULT;
	WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

	// Call the method
	HRESULT hr =
		pDispatch->lpVtbl->Invoke( pDispatch, dispid, riid, lcid, wFlags, &params, retval, &excepinfo, NULL );

	return hr;
}

int
BoolAXVoid( IDispatch* pDispatch, const unsigned short* methodname )
{
	// Calls a method that takes no arguments and returns bool
	DISPID	dispid;
	HRESULT hr = GetDispId( pDispatch, methodname, &dispid );
	if ( !CheckHR( hr, TEXT( "Error getting DISPID for BoolAXVoid" ) ) ) {
		return FALSE;
	}

	// Prepare and invoke the "AboutBox" method
	DISPPARAMS params = { NULL, NULL, 0, 0 };
	VARIANT	   retval;
	VariantInit( &retval );
	EXCEPINFO excepinfo;
	memset( &excepinfo, 0, sizeof( excepinfo ) );
	REFIID riid   = &IID_NULL;
	LCID   lcid   = LOCALE_USER_DEFAULT;
	WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

	// Call the method
	hr = pDispatch->lpVtbl->Invoke( pDispatch, dispid, riid, lcid, wFlags, &params, &retval, &excepinfo, NULL );

	if ( !CheckHR( hr, TEXT( "Failed to invoke BoolAXVoid method" ) ) ) {
		return 1;
	}

	// Release resources and clean up
	VariantClear( &retval );

	return 0;
}

int
InitializeActiveXControl( struct activexctrl* control )
{
	// Initialize COM libraries
	control->hr = CoInitialize( NULL );
	if ( !CheckHR( control->hr, TEXT( "CoInitialize failed" ) ) ) {
		return 1;
	}

	control->hr = OleInitialize( NULL );
	if ( !CheckHR( control->hr, TEXT( "OleInitialize failed" ) ) ) {
		return 1;
	}

	// Allocate storage for OleCreate
	control->plkbyt = NULL;
	control->hr	= CreateILockBytesOnHGlobal( NULL, TRUE, &control->plkbyt );
	if ( !CheckHR( control->hr, TEXT( "CreateILockBytesOnHGlobal failed" ) ) ) {
		return 1;
	}

	control->pStorage = NULL;
	control->hr	  = StgCreateDocfileOnILockBytes(
		      control->plkbyt, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &control->pStorage );
	if ( !CheckHR( control->hr, TEXT( "StgCreateDocfileOnILockBytes failed" ) ) ) {
		return 1;
	}

	// Instantiate the ActiveX control
	control->pActiveXControl = NULL;
	const CLSID clsid	 = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };
	control->hr		 = OleCreate( &clsid,
				      &IID_IOleObject,
				      OLERENDER_DRAW,
				      NULL,
				      NULL,
				      control->pStorage,
				      (LPVOID*)&control->pActiveXControl );
	if ( !CheckHR( control->hr, TEXT( "OleCreate failed" ) ) ) {
		return 1;
	}

	// Get the dispatch pointer
	control->hr = control->pActiveXControl->lpVtbl->QueryInterface(
		control->pActiveXControl, &IID_IDispatch, (LPVOID*)&control->pDispatch );
	if ( !CheckHR( control->hr, TEXT( "QueryInterface failed for IID_IDispatch" ) ) ) {
		return 1;
	}

	// Get the ITypeInfo interface for the IDispatch object
	control->hr = control->pDispatch->lpVtbl->GetTypeInfo(
		control->pDispatch, 0, LOCALE_USER_DEFAULT, &control->pTypeInfo );
	if ( !CheckHR( control->hr, TEXT( "Failed to get type information" ) ) ) {
		return 1;
	}

	// Obtain the type library of the ActiveX control
	UINT index;
	control->hr =
		control->pTypeInfo->lpVtbl->GetContainingTypeLib( control->pTypeInfo, &control->pTypeLib, &index );
	control->pTypeInfo->lpVtbl->Release( control->pTypeInfo );
	if ( !CheckHR( control->hr, TEXT( "Failed to obtain type library" ) ) ) {
		return 1;
	}

	return 0;
}

void
ReleaseActiveXControl( struct activexctrl* control )
{
	// Release resources
	control->pTypeLib->lpVtbl->Release( control->pTypeLib );
	control->pDispatch->lpVtbl->Release( control->pDispatch );
	control->pActiveXControl->lpVtbl->Release( control->pActiveXControl );
	control->pStorage->lpVtbl->Release( control->pStorage );
	control->plkbyt->lpVtbl->Release( control->plkbyt );
	CoUninitialize();
}

void
ResizeActiveXControl( IOleObject* pActiveXControl, LONG widthHimetric, LONG heightHimetric )
{
	// Set the size of the ActiveX control
	HRESULT hr = pActiveXControl->lpVtbl->SetExtent(
		pActiveXControl,
		DVASPECT_CONTENT,			   // Aspect to be resized (content aspect in this case)
		&(SIZEL){ widthHimetric, heightHimetric }  // New size in HIMETRIC units
	);

	if ( !CheckHR( hr, TEXT( "SetExtent failed" ) ) ) {
		return;
	}

	printf( "ActiveX control resized to %ldx%ld HIMETRIC units.\n", widthHimetric, heightHimetric );
}

int
main()
{
	struct activexctrl lmos;

	printf( "Initializing control...\n" );
	int result = InitializeActiveXControl( &lmos );
	if ( result ) {
		fprintf( stderr, "Could not initialize Lmos control.\n" );
		return -1;
	}

	// Resize window here
	LONG widthHimetric  = 30000;  // Example width
	LONG heightHimetric = 30000;  // Example height


	// Show window
	BoolAXVoid( lmos.pDispatch, L"ShowMarkingArea" );  // Displays the control

	HRESULT hr = lmos.pActiveXControl->lpVtbl->DoVerb(
		lmos.pActiveXControl, OLEIVERB_SHOW, NULL, (IOleClientSite*)lmos.pActiveXControl, 0, NULL, NULL );
	if ( !CheckHR( hr, TEXT( "Failed to display window" ) ) ) {
		return 1;
	}

	ResizeActiveXControl( lmos.pActiveXControl, widthHimetric, heightHimetric );

	// Run tests
	VARIANT varResult;
	StringActiveXHr( lmos.pDispatch, L"GetMarkingFilesPath", &varResult );
	if ( varResult.vt == VT_BSTR ) {
		wprintf( L"Marking Files Path: %s\n", varResult.bstrVal );
	} else {
		fprintf( stderr, TEXT( "Failed to invoke GetMarkingFilesPath method: wrong return type\n" ) );
		return 1;
	}
	VariantClear( &varResult );

	// String argument, returns bool
	printf( "Loading locket\n" );
	BoolAXVoid( lmos.pDispatch, L"CancelJob" );
	BoolAXVoid( lmos.pDispatch, L"ClearLayout" );
	/* BoolAXString(lmos.pDispatch, L"FileName2", L"V_cut_leaf_locket.VLM"); */

	// Display the About Box
	BoolAXVoid( lmos.pDispatch, L"AboutBox" );

	printf( "Sleeping\n" );
	while ( 1 ) {
		Sleep( 100 );
	}

	// Clean up
	printf( "Releasing control...\n" );
	ReleaseActiveXControl( &lmos );

	return 0;
}
