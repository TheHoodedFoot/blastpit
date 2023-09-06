#include <windows.h>  // Must be first to prevent Zig error

DEFINE_GUID( CLSID_IExample, 0x3E6FBC49, 0x6C3D, 0x4c10, 0x81, 0xDD, 0xDA, 0xD7, 0x4E, 0x7B, 0x39, 0xAC );
DEFINE_GUID( IID_IExample, 0xD81CCA6E, 0x4FEA, 0x49b3, 0xA8, 0x2E, 0x32, 0x13, 0x7F, 0x10, 0x17, 0x28 );

typedef struct
{
	// _Lmos_clientEx* lmosObject;
	void* lmosObject;
	IClassFactory*	    classFactory;
} t_lmos_struct;

void
printhr( HRESULT hr );

int
SetupCOM( t_lmos_struct* this );
void
ShutdownCOM( t_lmos_struct* this );
void
demo( t_lmos_struct* this );
