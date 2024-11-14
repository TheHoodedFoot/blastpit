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


int
call_activex( IDispatch* pDispatch, const unsigned short* methodname, VARIANT* retval )
{
	/** Calls an ActiveX control's method by name
	 * The method takes no arguments and returns a variant result
	 *
	 * @param retval Pointer to VARIANT to hold result
	 * @return HRESULT of method call
	 */

	DISPPARAMS params = { NULL, NULL, 0, 0 };
	VariantInit( retval );
	EXCEPINFO excepinfo;
	memset( &excepinfo, 0, sizeof( excepinfo ) );
	REFIID riid   = &IID_NULL;
	LCID   lcid   = LOCALE_USER_DEFAULT;
	WORD   wFlags = DISPATCH_METHOD | DISPATCH_PROPERTYGET;

    // Call the method
    DISPID dispid;
    HRESULT hr = GetDispId(pDispatch, methodname, &dispid);
    if (FAILED(hr)) {
        return hr;
    }
	hr =
		pDispatch->lpVtbl->Invoke( pDispatch, dispid, riid, lcid, wFlags, &params, retval, &excepinfo, NULL );

	return hr;
}

int call_activex_string(IDispatch* pDispatch, const wchar_t* methodName, BSTR argString, VARIANT* retval)
{
    /** Calls an ActiveX control's method by name and passes a wide string as an argument
     * The method takes one wide string argument and returns a variant result
     *
     * @param methodName Wide string representing the method name to call
     * @param argString  Wide string argument to pass to the method
     * @param retval     Pointer to VARIANT to hold result
     * @return HRESULT of method call
     */

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VariantInit(retval);
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));
    REFIID riid   = &IID_NULL;
    LCID   lcid   = LOCALE_USER_DEFAULT;
    WORD   wFlags = DISPATCH_METHOD;

    // Call the method
    DISPID dispid;
    HRESULT hr = GetDispId(pDispatch, methodName, &dispid);
    if (FAILED(hr)) {
        return hr;
    }

    VARIANTARG arg[1];
    arg[0].vt = VT_BSTR;
    arg[0].bstrVal = argString;

    params.rgvarg  = arg;
    params.cArgs   = 1;
    hr = pDispatch->lpVtbl->Invoke(pDispatch, dispid, riid, lcid, wFlags, &params, retval, &excepinfo, NULL);

    return hr;
}

int
main()
{
	struct activexctrl lmos;

    const char *xml = "<?xml version=\"1.0\"?><DRAWING UNIT=\"MM\"><ROOT ID=\"Blastpit\" WIDTH=\"120.0\" HEIGHT=\"120.0\"><LAYER NAME=\"bp_edd400\" HEIGHT_Z_AXIS=\"120.0\" COLOR=\"237,212,0\" /><GROUP ID=\"grect1\" USE_BOX=\"Y\" LAYER=\"bp_edd400\" LP=\"bp_edd400\" REF_POINT=\"CC\" HATCH=\"Y\" HP=\"bp_0_01\"><POLYLINE ID=\"rect1#edd400\" LAYER=\"bp_edd400\" LP=\"bp_edd400\"><POLYPOINT TYPE=\"LINE\">44.900517 69.601463</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">55.018406 69.601463</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">65.136292 69.601463</POLYPOINT><POLYPOINT TYPE=\"LINE\">75.254181 69.601463</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">75.254181 62.347126</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">75.254181 55.092788999999996</POLYPOINT><POLYPOINT TYPE=\"LINE\">75.254181 47.838454999999996</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">65.136292 47.838454999999996</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">55.018406 47.838454999999996</POLYPOINT><POLYPOINT TYPE=\"LINE\">44.900517 47.838454999999996</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">44.900517 55.092788999999996</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">44.900517 62.347126</POLYPOINT><POLYPOINT TYPE=\"LINE\">44.900517 69.601463</POLYPOINT><POLYPOINT TYPE=\"LINE\">44.900517 69.601463</POLYPOINT></POLYLINE></GROUP></ROOT></DRAWING>";

	printf( "Initializing control...\n" );
	int result = InitializeActiveXControl( &lmos );
	if ( result ) {
		fprintf( stderr, "Could not initialize Lmos control.\n" );
		return 1;
	}

	// Clear any existing drawing
	VARIANT varResult;
	call_activex( lmos.pDispatch, L"CancelJob", &varResult );
	call_activex( lmos.pDispatch, L"ClearLayout", &varResult );

    // Load the XML drawing from the const string
	printf( "Loading locket\n" );
	call_activex_string(lmos.pDispatch, L"FileName2", L"V_cut_leaf_locket.VLM", &varResult);
    if (varResult.vt == VT_BOOL) {
        printf("FileName2 method returned: %s\n", varResult.boolVal ? "True" : "False");
    } else {
        fprintf(stderr, TEXT("Failed to invoke FileName2 method: wrong return type\n"));
        return 1;
    }
    VariantClear(&varResult);

	// Display the About Box
	call_activex( lmos.pDispatch, L"AboutBox", &varResult );

	// Clean up
	printf( "Releasing control...\n" );
	ReleaseActiveXControl( &lmos );

	return 0;
}
