/**
 * @file activex_example.cpp
 * @brief This program demonstrates how to instantiate an ActiveX control, obtain its IDispatch pointer, and call its
 * "AboutBox" method.
 */

// Include necessary headers
#include <iostream>
#include <ole2.h>

// Use standard namespaces
using namespace std;

/**
 * @brief Helper function to check the result of various operations and print error messages accordingly.
 * @param hr The HRESULT value returned by the operation.
 * @param message A string describing the operation being checked.
 * @return True if the operation succeeded, false otherwise.
 */
bool
CheckHR( HRESULT hr, const char* message )
{
	if ( FAILED( hr ) ) {
		fprintf( stderr, "%s: 0x%08lX\n", message, hr );
		return false;
	}
	return true;
}

int
main( int argc, char* argv[] )
{
	// Silence warnings about unused arguments
	void( argc ), void( argv );

	// Initialize COM libraries
	HRESULT hr = CoInitialize( NULL );
	if ( !CheckHR( hr, "CoInitialize failed" ) ) {
		return 1;
	}

	hr = OleInitialize( NULL );
	if ( !CheckHR( hr, "OleInitialize failed" ) ) {
		return 1;
	}

	// Allocate storage for OleCreate
	ILockBytes* plkbyt = NULL;
	hr		   = CreateILockBytesOnHGlobal( NULL, TRUE, &plkbyt );

	IStorage* pStorage = NULL;
	hr = StgCreateDocfileOnILockBytes( plkbyt, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pStorage );

	// Instantiate the ActiveX control
	IOleObject* pActiveXControl = nullptr;
	const CLSID CLSID_LMOS = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };
	hr = OleCreate( CLSID_LMOS, IID_IOleObject, OLERENDER_DRAW, NULL, NULL, pStorage, (void**)&pActiveXControl );
	if ( !CheckHR( hr, "OleCreate failed" ) ) {
		return 1;
	}

	// Get the dispatch pointer
	IDispatch* pDispatch;
	hr = pActiveXControl->QueryInterface( IID_IDispatch, reinterpret_cast<void**>( &pDispatch ) );
	if ( !CheckHR( hr, "QueryInterface failed for IID_IDispatch" ) ) {
		return 1;
	}

	// Get the ITypeInfo interface for the IDispatch object
	ITypeInfo* pTypeInfo;
	hr = pDispatch->GetTypeInfo( 0, LOCALE_USER_DEFAULT, &pTypeInfo );
	if ( !CheckHR( hr, "Failed to get type information" ) ) {
		return 1;
	}

	// Obtain the type library of the ActiveX control
	ITypeLib* pTypeLib;
	UINT	  index;
	hr = pTypeInfo->GetContainingTypeLib( &pTypeLib, &index );
	pTypeInfo->Release();
	if ( !CheckHR( hr, "Failed to obtain type library" ) ) {
		return 1;
	}

	// Obtain the DISPID for the "AboutBox" method
	BSTR   bstrMethodName = SysAllocString( L"AboutBox" );
	DISPID dispidAboutBoxMethod;
	hr = pDispatch->GetIDsOfNames( IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispidAboutBoxMethod );
	SysFreeString( bstrMethodName );
	if ( !CheckHR( hr, "Failed to get DISPID for AboutBox method" ) ) {
		return 1;
	}

	// Prepare and invoke the "AboutBox" method
	DISPPARAMS params = { NULL, NULL, 0, 0 };
	VARIANT	   retVal;
	VariantInit( &retVal );
	UINT	  argerr = 0;
	EXCEPINFO excepinfo;
	memset( &excepinfo, 0, sizeof( excepinfo ) );
	REFIID riid   = IID_NULL;
	LCID   lcid   = LOCALE_USER_DEFAULT;
	WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

	// Call the method
	hr = pDispatch->Invoke( dispidAboutBoxMethod, riid, lcid, wFlags, &params, &retVal, &excepinfo, &argerr );

	if ( !CheckHR( hr, "Failed to invoke AboutBox method" ) ) {
		DWORD dwError = HRESULT_FROM_WIN32( GetLastError() );
		fprintf( stderr,
			 "Failed to invoke AboutBox method: HRESULT=0x%08lX, Win32Error=0x%08lX\n",
			 hr,
			 dwError );
		return 1;
	}

	// Release resources and clean up
	VariantClear( &retVal );
	pTypeLib->Release();
	pDispatch->Release();
	pActiveXControl->Release();
	pStorage->Release();
	plkbyt->Release();
	CoUninitialize();

	return 0;
}
