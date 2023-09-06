#include <windows.h>  // Must be first to prevent Zig error

#include "sc_samlight_client_ctrl_ex_type_lib.h"
// #include <initguid.h>
// #include <objbase.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unknwn.h>


// SCAPS.ScSamlightClientCtrlEx
DEFINE_GUID( CLSID_IExample, 0x3E6FBC49, 0x6C3D, 0x4c10, 0x81, 0xDD, 0xDA, 0xD7, 0x4E, 0x7B, 0x39, 0xAC );
DEFINE_GUID( IID_IExample, 0xD81CCA6E, 0x4FEA, 0x49b3, 0xA8, 0x2E, 0x32, 0x13, 0x7F, 0x10, 0x17, 0x28 );

typedef struct
{
	_Samlight_clientEx* scapsObject;
	IClassFactory*	    classFactory;
} t_scaps_struct;


void demo( t_scaps_struct* this );
void printhr( HRESULT hr );

int  SetupCOM( t_scaps_struct* this );
void ShutdownCOM( t_scaps_struct* this );
void disableViewUpdate( _Samlight_clientEx* this );
void enableViewUpdate( _Samlight_clientEx* this );
HRESULT
createRectangle( _Samlight_clientEx* this,
		 const char* name,
		 const char* father,
		 double	     x,
		 double	     y,
		 double	     width,
		 double	     height,
		 int	     pen );

void demo( t_scaps_struct* this );
