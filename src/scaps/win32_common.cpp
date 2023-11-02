#include <windows.h>  // Must be first to prevent Zig error

#include <getopt.h>
#include <initguid.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unknwn.h>


// ░█▀▄░█▀▀░█▀▄░█░█░█▀▀░░░█░█░█▀▀░█░░░█▀█░█▀▀░█▀▄░█▀▀
// ░█░█░█▀▀░█▀▄░█░█░█░█░░░█▀█░█▀▀░█░░░█▀▀░█▀▀░█▀▄░▀▀█
// ░▀▀░░▀▀▀░▀▀░░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀▀▀░▀░░░▀▀▀░▀░▀░▀▀▀

void
printhr( HRESULT hr )
{
	printf( "hr = 0x%x\n", hr );
}


// ░█░█░▀█▀░▀█▀░█░░░▀█▀░▀█▀░▀█▀░█▀▀░█▀▀
// ░█░█░░█░░░█░░█░░░░█░░░█░░░█░░█▀▀░▀▀█
// ░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀

// BSTR
// StringToBSTR( const char* string )
// {  // Allocates a BSTR from a char string (must be freed with SysFreeString)
//
// 	char* wide_string = calloc( 1, ( strlen( string ) + 1 ) * 2 );
// 	mbstowcs( (wchar_t*)wide_string, string, strlen( string ) );
// 	BSTR bstr_string = SysAllocString( (wchar_t*)wide_string );
// 	free( wide_string );
// 	return ( bstr_string );
// }
