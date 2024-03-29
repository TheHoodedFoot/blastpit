#include <windows.h>  // Must be first to prevent Zig error

#include <initguid.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unknwn.h>

#include "lmos.h"


// ░█▀▄░█▀▀░█▀▄░█░█░█▀▀░░░█░█░█▀▀░█░░░█▀█░█▀▀░█▀▄░█▀▀
// ░█░█░█▀▀░█▀▄░█░█░█░█░░░█▀█░█▀▀░█░░░█▀▀░█▀▀░█▀▄░▀▀█
// ░▀▀░░▀▀▀░▀▀░░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀▀▀░▀░░░▀▀▀░▀░▀░▀▀▀

void
printhr( HRESULT hr )
{
	printf( "hr = 0x%x\n", hr );
}


// ░█▀▀░█▀█░█▄█
// ░█░░░█░█░█░█
// ░▀▀▀░▀▀▀░▀░▀

int
SetupCOM( t_lmos_struct* this )
{
	HRESULT hr;

	// We must initialize OLE before we do anything with COM objects.
	if ( CoInitialize( 0 ) ) {
		MessageBox( 0, "Can't initialize COM", "CoInitialize error", MB_OK | MB_ICONEXCLAMATION );
		return ( 0 );
	}

	// Get IExample.DLL's IClassFactory
	if ( ( hr = CoGetClassObject( &CLSID_IExample,
				      CLSCTX_INPROC_SERVER,
				      0,
				      &IID_IClassFactory,
				      (void**)&( this->classFactory ) ) ) ) {
		MessageBox( 0, "Can't get IClassFactory", "CoGetClassObject error", MB_OK | MB_ICONEXCLAMATION );
		CoUninitialize();
		return ( 0 );
	}

	// Create an IExample object
	if ( ( hr = this->classFactory->lpVtbl->CreateInstance(
		       this->classFactory, 0, &IID_IExample, (void**)&( this->lmosObject ) ) ) ) {
		this->classFactory->lpVtbl->Release( this->classFactory );
		MessageBox( 0, "Can't create IExample object", "CreateInstance error", MB_OK | MB_ICONEXCLAMATION );
		CoUninitialize();
		return ( 0 );
	}

	// Release the IClassFactory. We don't need it now that we have the one
	// IExample we want
	this->classFactory->lpVtbl->Release( this->classFactory );

	return 1;
}

void
ShutdownCOM( t_lmos_struct* this )
{
	// Release the IExample now that we're done with it
	this->lmosObject->lpVtbl->Release( this->lmosObject );

	// When finally done with OLE, free it
	CoUninitialize();
}


// ░█░█░▀█▀░▀█▀░█░░░▀█▀░▀█▀░▀█▀░█▀▀░█▀▀
// ░█░█░░█░░░█░░█░░░░█░░░█░░░█░░█▀▀░▀▀█
// ░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀

BSTR
StringToBSTR( const char *string )
{ // Allocates a BSTR from a char string (must be freed with SysFreeString

	char* wide_string = calloc( 1, ( strlen( string ) + 1 ) * 2 );
	mbstowcs( (wchar_t*)wide_string, string, strlen( string ) );
	BSTR bstr_string = SysAllocString( (wchar_t*)wide_string );
	free( wide_string );
	return (bstr_string);
}


// ░█▀▀░█▀▀░█▀█░█▄█░█▀▀░▀█▀░█▀▄░▀█▀░█▀▀░█▀▀
// ░█░█░█▀▀░█░█░█░█░█▀▀░░█░░█▀▄░░█░░█▀▀░▀▀█
// ░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░░▀░░▀░▀░▀▀▀░▀▀▀░▀▀▀

int
main( int argc, char** argv )
{
	t_lmos_struct this;

	// Setup COM
	if ( SetupCOM( &this ) == 0 ) {
		exit( 71 );
	}

	ShutdownCOM( &this );

	return ( 0 );
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
