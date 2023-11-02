#include <windows.h>  // Must be first to prevent error when compiling with zig cc

#include <initguid.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unknwn.h>


DEFINE_GUID( CLSID_Lmos, 0x18213698, 0xA9C9, 0x11D1, 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 );


#undef  INTERFACE
#define INTERFACE   ILmos
DECLARE_INTERFACE_ (ILmos, IUnknown)
{
   // IUnknown functions
   STDMETHOD  (QueryInterface)         (THIS_ REFIID, void **) PURE;
   STDMETHOD_ (ULONG, AddRef)          (THIS) PURE;
   STDMETHOD_ (ULONG, Release)         (THIS) PURE;
   // IDispatch functions
   STDMETHOD_ (ULONG, GetTypeInfoCount)(THIS_ UINT *) PURE;
   STDMETHOD_ (ULONG, GetTypeInfo)     (THIS_ UINT, LCID, ITypeInfo **) PURE;
   STDMETHOD_ (ULONG, GetIDsOfNames)   (THIS_ REFIID, LPOLESTR *, 
                                        UINT, LCID, DISPID *) PURE;
   STDMETHOD_ (ULONG, Invoke)          (THIS_ DISPID, REFIID, 
                                        LCID, WORD, DISPPARAMS *,
                                        VARIANT *, EXCEPINFO *, UINT *) PURE;
   // Extra functions
   STDMETHOD  (AboutBox)              (THIS) PURE;
};


typedef struct
{
	ILmos* lmosObject;
	IClassFactory*	    classFactory;
} t_lmos_struct;


int
SetupCOM( t_lmos_struct* this )
{
	HRESULT hr;

	// We must initialize OLE before we do anything with COM objects.
	if ( CoInitialize( 0 ) ) {
		MessageBox( 0, "Can't initialize COM", "CoInitialize error", MB_OK | MB_ICONEXCLAMATION );
		return ( 0 );
	}

	// Get the IClassFactory
	if ( ( hr = CoGetClassObject( &CLSID_Lmos,
				      CLSCTX_INPROC_SERVER,
				      0,
				      &IID_IUnknown,
				      (void**)&( this->classFactory ) ) ) ) {
		MessageBox( 0, "Can't get IClassFactory", "CoGetClassObject error", MB_OK | MB_ICONEXCLAMATION );
		CoUninitialize();
		return ( hr );
	}

	// Create the instance
	if ( ( hr = this->classFactory->lpVtbl->CreateInstance(
		       this->classFactory, 0, &IID_IUnknown, (void**)&( this->lmosObject ) ) ) ) {
		this->classFactory->lpVtbl->Release( this->classFactory );
		MessageBox( 0, "Can't create IExample object", "CreateInstance error", MB_OK | MB_ICONEXCLAMATION );
		CoUninitialize();
		return ( hr );
	}

	// Release the IClassFactory. We don't need it now that we have the instance.
	this->classFactory->lpVtbl->Release( this->classFactory );

	return 1;
}

void
ShutdownCOM( t_lmos_struct* this )
{
	// Release the instance now that we're done with it
	IUnknown *pUnknown = (IUnknown *)this->lmosObject;
	pUnknown->lpVtbl->Release( pUnknown );

	// When finally done with OLE, free it
	CoUninitialize();
}


int
main( int argc, char** argv )
{
	t_lmos_struct this;

	// Setup COM
	if ( SetupCOM( &this ) == 0 ) {
		printf("SetupCOM failed.\n");
		exit( 71 );
	}
	printf("SetupCOM completed successfully.\n");

	this.lmosObject->lpVtbl->AboutBox(this.lmosObject);

	ShutdownCOM( &this );

	return ( 0 );
}

// Compile with:
//zig cc -target x86-windows-gnu -fno-stack-protector -o /home/thf/projects/blastpit/build/lmos.exe -I/home/thf/projects/blastpit/src/lmos /home/thf/projects/blastpit/src/lmos/lmos_minimal_test.c -lole32 -loleaut32 -luuid -lwsock32
