#include <windows.h>  // Must be first to prevent Zig error

DEFINE_GUID( CLSID_Lmos, 0x18213698, 0xA9C9, 0x11D1, 0xA2, 0x20, 0x00, 0x60, 0x97, 0x30, 0x58, 0xF6 );

typedef struct
{
	void* lmosObject;
	IClassFactory*	    classFactory;
} t_lmos_struct;

typedef interface _Lmos_client _Lmos_client;

typedef struct _Lmos_clientVtbl
{
	BEGIN_INTERFACE

	HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
    			_Lmos_client * This,
    			/* [in] */ REFIID riid,
    			/* [annotation][iid_is][out] */ 
    			_COM_Outptr_  void **ppvObject);

	ULONG ( STDMETHODCALLTYPE *AddRef )( 
    			_Lmos_client * This);

	ULONG ( STDMETHODCALLTYPE *Release )( 
    			_Lmos_client * This);

	/* [id] */ HRESULT ( STDMETHODCALLTYPE *ScIsRunning )( 
    			_Lmos_client * This,
    			/* [retval][out] */ long *res);

	/* [id] */ HRESULT ( STDMETHODCALLTYPE *ScSetEntityOutline3D )( 
    			_Lmos_client * This,
    			BSTR EntityName,
    			double MinX,
    			double MinY,
    			double MaxX,
    			double MaxY,
    			double MinZ,
    			double MaxZ,
    			long Flags);

	END_INTERFACE
} _Lmos_clientVtbl;

interface _Lmos_client
{
	CONST_VTBL struct _Lmos_clientVtbl *lpVtbl;
};

void
printhr( HRESULT hr );

int
SetupCOM( t_lmos_struct* this );
void
ShutdownCOM( t_lmos_struct* this );
void
demo( t_lmos_struct* this );
