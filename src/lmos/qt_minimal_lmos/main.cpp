#include "lmosactxlib.h"
#include <QApplication>

#include <iostream>
#include <ole2.h>
using namespace std;

#include <QDebug>
#include <QUuid>

#include <comdef.h>
#include <windows.h>


// Our console test that does not work
void
consoletest2( IDispatch* pDispatch )
{
	HRESULT hr;

	hr = CoInitialize( NULL );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "CoInitialize failed: 0x%08lX\n", hr );
		return;
	}

	hr = OleInitialize( NULL );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "OleInitialize failed: 0x%08lX\n", hr );
		return;
	}

	fprintf( stderr, "pDispatch: %p\n", pDispatch );

	HGLOBAL hGlobal = GlobalAlloc( GMEM_MOVEABLE, 0 );
	if ( hGlobal == NULL ) {
		// Handle error
		fprintf( stderr, "GlobalAlloc failed: %08lx\n", hr );
		return;
	}

	ILockBytes* pLockBytes = NULL;
	hr		       = CreateILockBytesOnHGlobal( hGlobal, TRUE, &pLockBytes );
	if ( FAILED( hr ) ) {
		// Handle error
		fprintf( stderr, "CreateILockBytesOnHGlobal failed: %08lx\n", hr );
		return;
	}

	IStorage* pStorage = NULL;
	hr = StgCreateDocfileOnILockBytes( pLockBytes, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStorage );
	if ( FAILED( hr ) ) {
		// Handle error
		fprintf( stderr, "StgCreateDocfileOnILockBytes failed: %08lx\n", hr );
		return;
	}

	// Instantiate the ActiveX control.
	IOleObject* pControl = nullptr;
	// IUnknown** ppUnkn   = &pUnknown;

	const CLSID CLSID_LMOS = { 0x18213698, 0xA9C9, 0x11D1, { 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 } };

	hr = OleCreate( CLSID_LMOS, IID_IOleObject, OLERENDER_DRAW, NULL, NULL, pStorage, (void**)&pControl );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "OleCreate failed: %08lx\n", hr );
		return;
	}

	//  	   IOleClientSite* pClientSite = NULL;
	//  	   hr = OleCreateDefaultHandler(CLSID_LMOS, NULL, IID_IOleClientSite, (void**)&pClientSite);
	//  	   if (FAILED(hr)) {
	// fprintf( stderr, "OleCreateDefaultHandler failed: %08lx\n", hr );
	// return;
	//  	   }
	//
	//  	   hr = pControl->SetClientSite(pClientSite);
	//  	   if (FAILED(hr)) {
	// fprintf( stderr, "SetClientSite failed: %08lx\n", hr );
	// return;
	//  	   }
	//
	//
	//  	   hr = pControl->DoVerb(OLEIVERB_SHOW, NULL, pClientSite, -1, NULL, NULL);
	//  	   if (FAILED(hr)) {
	// fprintf( stderr, "DoVerb failed: %08lx\n", hr );
	// return;
	//  	   }

	// hr = CoCreateInstance( CLSID_LMOS, 0, CLSCTX_INPROC_SERVER, IID_IOleObject, reinterpret_cast<void**>( &pOle )
	// );

	fprintf( stderr, "CLSCTX_SERVER= %d\n", CLSCTX_SERVER );

	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed to instantiate ActiveX control: %08lx\n", hr );
		return;
	} else {
		fprintf( stderr, "CoCreateInstance succeeded...\n" );
		fprintf( stderr, "pControl: %p\n", pControl );
		// fprintf( stderr, "pOleown: %p\n", pOleown );
	}

	// Get the dispatch pointer
	IDispatch* pDispatch2;
	hr = pControl->QueryInterface( IID_IDispatch, reinterpret_cast<void**>( &pDispatch2 ) );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "QueryInterface failed for IID_IDispatch: %08lx\n", hr );
		return;
	} else {
		fprintf( stderr, "Obtained IDispatch...\n" );
	}

	fprintf( stderr, "pDispatch: %p\n", pDispatch );
	fprintf( stderr, "pDispatch2: %p\n", pDispatch2 );

	pDispatch = pDispatch2;

	// Get the ITypeInfo interface for the IDispatch object.
	ITypeInfo* pTypeInfo;
	hr = pDispatch->GetTypeInfo( 0, LOCALE_USER_DEFAULT, &pTypeInfo );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed to get type information: %08lx\n", hr );
		return;
	}

	// Obtain the type library of the ActiveX control.
	ITypeLib* pTypeLib;
	UINT	  index;
	hr = pTypeInfo->GetContainingTypeLib( &pTypeLib, &index );
	pTypeInfo->Release();
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed to obtain type library: %08lx\n", hr );
		return;
	}

	// Step 6: Obtain the DISPID (Dispatch Identifier) for the "AboutBox" method.
	BSTR   bstrMethodName = SysAllocString( L"AboutBox" );
	DISPID dispidMethod;
	hr = pDispatch->GetIDsOfNames( IID_NULL, &bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispidMethod );
	SysFreeString( bstrMethodName );  // Release the allocated BSTR
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed to get DISPID for method: %08lx\n", hr );
		return;
	}

	// Step 7: Prepare and invoke the "AboutBox" method.
	DISPPARAMS params = { NULL, NULL, 0, 0 };
	VARIANT	   ret;
	VariantInit( &ret );
	UINT	  argerr = 0;
	EXCEPINFO excepinfo;
	memset( &excepinfo, 0, sizeof( excepinfo ) );
	REFIID myid   = IID_NULL;
	LCID   mylcid = LOCALE_USER_DEFAULT;
	WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

	// call the method
	hr = pDispatch->Invoke( dispidMethod, myid, mylcid, wFlags, &params, &ret, &excepinfo, &argerr );

	if ( FAILED( hr ) ) {
		DWORD dwError = HRESULT_FROM_WIN32( GetLastError() );
		fprintf( stderr, "Failed to invoke method: HRESULT=0x%08lX, Win32Error=0x%08lX\n", hr, dwError );
		return;
	}

	// Step 8: Release resources and clean up.
	pControl->Release();
	// pClientSite->Release();
	pStorage->Release();
	pLockBytes->Release();
	VariantClear( &ret );
	pTypeLib->Release();
	pDispatch->Release();  // May be called automatically by destructor?
	GlobalFree( hGlobal );
	CoUninitialize();
	return;
}

int
main( int argc, char* argv[] )
{
	QApplication a( argc, argv );

	LMOSACTXLib::LMOSActX lmos;
	// lmos.AboutBox();

	IDispatch* disp = 0;
	lmos.queryInterface( QUuid( IID_IDispatch ), (void**)&disp );
	if ( disp ) {
		// For the test, we just call consoletest()
		consoletest2( disp );
	}

	// return a.exec();
}
