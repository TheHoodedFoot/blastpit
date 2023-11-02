/**
 * @file main.c
 * @brief This program demonstrates how to instantiate an ActiveX control, obtain its IDispatch pointer, and call the
 * "AboutBox" method.
 */

#include <ole2.h>
#include <stdio.h>
#include <windows.h>

/* Check if the given HRESULT indicates success or failure, print error message on failure */
BOOL
CheckHR( HRESULT hr, LPCTSTR szMessage )
{
	if ( FAILED( hr ) ) {
		fprintf( stderr, TEXT( "%s: 0x%08lX\n" ), szMessage, hr );
		return FALSE;
	}
	return TRUE;
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

	// Obtain the DISPID for the "AboutBox" method
	BSTR   bstrMethodName = SysAllocString( L"AboutBox" );
	DISPID dispidAboutBoxMethod;
	hr = pDispatch->lpVtbl->GetIDsOfNames(
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
	pTypeLib->lpVtbl->Release( pTypeLib );
	pDispatch->lpVtbl->Release( pDispatch );
	pActiveXControl->lpVtbl->Release( pActiveXControl );
	pStorage->lpVtbl->Release( pStorage );
	plkbyt->lpVtbl->Release( plkbyt );
	CoUninitialize();

	return 0;
}
