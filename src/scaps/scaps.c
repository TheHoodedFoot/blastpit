#include <windows.h>  // Must be first to prevent Zig error

#include "sc_samlight_client_ctrl_ex_type_lib.h"
#include <getopt.h>
#include <initguid.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unknwn.h>

#include "scaps.h"

typedef struct
{
	int   pen_number;
	float pen_power;
	float pen_speed;
	float pen_frequency;
	float pen_ipg;
	char  pen_name[256];
} t_pen_properties;


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
SetupCOM( t_scaps_struct* this )
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
		       this->classFactory, 0, &IID_IExample, (void**)&( this->scapsObject ) ) ) ) {
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
ShutdownCOM( t_scaps_struct* this )
{
	// Release the IExample now that we're done with it
	this->scapsObject->lpVtbl->Release( this->scapsObject );

	// When finally done with OLE, free it
	CoUninitialize();
}


// ░█░█░▀█▀░▀█▀░█░░░▀█▀░▀█▀░▀█▀░█▀▀░█▀▀
// ░█░█░░█░░░█░░█░░░░█░░░█░░░█░░█▀▀░▀▀█
// ░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀

BSTR
StringToBSTR( const char* string )
{  // Allocates a BSTR from a char string (must be freed with SysFreeString)

	char* wide_string = calloc( 1, ( strlen( string ) + 1 ) * 2 );
	mbstowcs( (wchar_t*)wide_string, string, strlen( string ) );
	BSTR bstr_string = SysAllocString( (wchar_t*)wide_string );
	free( wide_string );
	return ( bstr_string );
}

void
disableViewUpdate( _Samlight_clientEx* this )
{  // Prevents the 2D view from updating

	long currentmode;
	// _Samlight_clientEx_ScGetMode( this, &currentmode );
	// currentmode |= scComSAMLightClientCtrlModeFlagDontUpdateView;
	// _Samlight_clientEx_ScSetMode( this, currentmode );
	// _Samlight_clientEx_ScSetMode( this, 2 );
	currentmode =
		scComSAMLightClientCtrlModeFlagDontUpdateView | scComSAMLightClientCtrlDoubleDataIdFlagDontUpdateView;
	_Samlight_clientEx_ScSetMode( this, currentmode );
}

void
enableViewUpdate( _Samlight_clientEx* this )
{  // Enables the 2D view to update

	int currentmode;
	// _Samlight_clientEx_ScGetMode( this, &currentmode );
	// currentmode &= ~( scComSAMLightClientCtrlModeFlagDontUpdateView );
	// _Samlight_clientEx_ScSetMode( this, currentmode );
	_Samlight_clientEx_ScSetMode( this, 0 );
	_Samlight_clientEx_ScExecCommand( this, scComSAMLightClientCtrlExecCommandUpdateViewNow );
}

void
importSVG( _Samlight_clientEx* this, const char* filename )
{
	BSTR bstr_filename = StringToBSTR( filename );
	BSTR bstr_svg	   = SysAllocString( L"svg" );
	_Samlight_clientEx_ScImport( this, bstr_svg, bstr_filename, bstr_svg, 0, 0 );
	fprintf( stdout, "%s: this = %p, filename = %s\n", __func__, this, filename );
	SysFreeString( bstr_svg );
	SysFreeString( bstr_filename );
}

int
getPen( _Samlight_clientEx* this, const int pennumber )
{  // Retrieves the pen info. Returns >0 if error at any step

	int ret;

	ret = _Samlight_clientEx_ScSetPen( this, pennumber );

	BSTR bstr_penname = SysAllocString( L"" );
	ret += _Samlight_clientEx_ScGetStringValue(
		this, scComSAMLightClientCtrlStringValueTypeCurrentPenName, &bstr_penname );

	double markspeed, frequency, pulselength, laserpower, ipg;
	ret += _Samlight_clientEx_ScGetDoubleValue( this, scComSAMLightClientCtrlDoubleValueTypeMarkSpeed, &markspeed );
	ret += _Samlight_clientEx_ScGetDoubleValue( this, scComSAMLightClientCtrlDoubleValueTypeFrequency, &frequency );
	ret += _Samlight_clientEx_ScGetDoubleValue(
		this, scComSAMLightClientCtrlDoubleValueTypeLaserPower, &laserpower );
	ret += _Samlight_clientEx_ScGetDoubleValue( this, scComSAMLightClientCtrlDoubleValueTypeIPGPulseLength, &ipg );

	printf( "%d, %f, %f, %f, %f, %S\n", pennumber, laserpower, markspeed, frequency, ipg, bstr_penname );

	SysFreeString( bstr_penname );

	return ret;
}

void
setPen( _Samlight_clientEx* this,
	const int   pennumber,
	const float power,
	const float speed,
	const float frequency,
	const float ipg,
	const char* name )
{  // Sets the values of a pen

	int ret;

	BSTR bstr_penname = StringToBSTR( name );

	printf( "setPen: %d, %f, %f, %f, %f, %s\n", pennumber, power, speed, frequency, ipg, name );

	ret = _Samlight_clientEx_ScSetPen( this, pennumber );
	// fprintf( stdout, "Return value from ScSetPen: %d\n", ret );
	// ret = _Samlight_clientEx_ScImport( this, bstr_svg, bstr_filename, bstr_svg, 0, 0 );

	ret = _Samlight_clientEx_ScSetStringValue(
		this, scComSAMLightClientCtrlStringValueTypeCurrentPenName, bstr_penname );
	// fprintf( stdout, "Return value from ScSetPen: %d\n", ret );

	ret = _Samlight_clientEx_ScSetDoubleValue(
		this, scComSAMLightClientCtrlDoubleValueTypeLaserPower, (double)power );
	// fprintf( stdout, "Return value from set TypeLaserPower: %d\n", ret );

	ret = _Samlight_clientEx_ScSetDoubleValue(
		this, scComSAMLightClientCtrlDoubleValueTypeMarkSpeed, (double)speed );
	// fprintf( stdout, "Return value from set TypeMarkSpeed: %d\n", ret );

	ret = _Samlight_clientEx_ScSetDoubleValue(
		this, scComSAMLightClientCtrlDoubleValueTypeFrequency, (double)frequency );
	// fprintf( stdout, "Return value from set TypeFrequency: %d\n", ret );

	ret = _Samlight_clientEx_ScSetDoubleValue(
		this, scComSAMLightClientCtrlDoubleValueTypeIPGPulseLength, (double)ipg );

	// fprintf( stdout, "Return value from set TypePulseLength: %d\n", ret );

	SysFreeString( bstr_penname );
}


// ░█▀▀░█▀▀░█▀█░█▄█░█▀▀░▀█▀░█▀▄░▀█▀░█▀▀░█▀▀
// ░█░█░█▀▀░█░█░█░█░█▀▀░░█░░█▀▄░░█░░█▀▀░▀▀█
// ░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░░▀░░▀░▀░▀▀▀░▀▀▀░▀▀▀

HRESULT
createRectangle( _Samlight_clientEx* this,
		 const char* name,
		 const char* father,
		 double	     x,
		 double	     y,
		 double	     width,
		 double	     height,
		 int	     pen )
{
	BSTR bstr_rectangle = SysAllocString( L"ScRectangle2D" );
	BSTR bstr_empty	    = SysAllocString( L"" );

	// scaps expects wide strings, so we must convert them with mbstowcs()
	char* wide_name = calloc( 1, ( strlen( name ) + 1 ) * 2 );
	mbstowcs( (wchar_t*)wide_name, name, strlen( name ) );
	BSTR bstr_name = SysAllocString( (wchar_t*)wide_name );
	free( wide_name );

	char* wide_father = calloc( 1, ( strlen( father ) + 1 ) * 2 );
	mbstowcs( (wchar_t*)wide_father, father, strlen( father ) );
	BSTR bstr_father = SysAllocString( (wchar_t*)wide_father );
	free( wide_father );

	BSTR* b_father;

	if ( strlen( father ) == 0 ) {
		b_father = &bstr_empty;
	} else {
		b_father = &bstr_father;
	}
	// Create a rectangle
	int ret = _Samlight_clientEx_ScSetStringValue(
		this, scComSAMLightClientCtrlStringValueTypeCreateEntity, bstr_rectangle );
	ret = _Samlight_clientEx_ScCreateEntity(
		this, bstr_name, *b_father, pen, x, y, 0, width, height, 0, bstr_empty );

	SysFreeString( bstr_name );
	SysFreeString( bstr_father );
	SysFreeString( bstr_empty );
	SysFreeString( bstr_rectangle );

	return ret;
}

t_pen_properties
GetNextPen()
{  // Read the next pen from the file or EOF

	t_pen_properties pen;
	pen.pen_number = 0;  // 0 signifies to calling function that pen is invalid
	char* line;
	char* token;
	int   i;
	char  buffer[256];

	line = fgets( buffer, sizeof( buffer ), stdin );

	// If the line is empty, return EOF

	if ( line == NULL ) {
		return pen;
	}

	// Remove the newline character from the end of the string

	line[strlen( line ) - 1] = '\0';

	// Parse the CSV string

	token = strtok( line, "," );
	i     = 0;
	while ( token != NULL ) {
		switch ( i ) {
			case 0:
				pen.pen_number = atoi( token );
				break;
			case 1:
				pen.pen_power = atof( token );
				break;
			case 2:
				pen.pen_speed = atof( token );
				break;
			case 3:
				pen.pen_frequency = atof( token );
				break;
			case 4:
				pen.pen_ipg = atof( token );
				break;
			case 5: {
				int len = strlen( token );
				if ( len >= sizeof( pen.pen_name ) ) {
					len = sizeof( pen.pen_name ) - 1;
				}
				memcpy( pen.pen_name, token, len );
				pen.pen_name[len] = '\0';
				break;
			}
		}
		i++;
		token = strtok( NULL, "," );
	}

	return pen;
}

void
UploadPens( t_scaps_struct* this )
{
	t_pen_properties pen = GetNextPen();
	// printf( "pen.pen_number = %d\n", pen.pen_number );
	while ( pen.pen_number > 0 ) {
		// printf( "UploadPens: %d, %f, %f, %f, %f, %s\n",
		// 	pen.pen_number,
		// 	pen.pen_power,
		// 	pen.pen_speed,
		// 	pen.pen_frequency,
		// 	pen.pen_ipg,
		// 	pen.pen_name );
		setPen( this->scapsObject,
			pen.pen_number,
			pen.pen_power,
			pen.pen_speed,
			pen.pen_frequency,
			pen.pen_ipg,
			pen.pen_name );
		pen = GetNextPen();
	}
}

// DownloadPens: Read the pens from Scaps and output them to STDOUT
void
DownloadPens( t_scaps_struct* this )
{
	for ( int i = 1; i < 255; i++ ) {
		getPen( this->scapsObject, i );
	}
}

// TestRectangles: Create some random rectangles to test Scaps connection
void
TestRectangles( t_scaps_struct* this )
{
	unsigned int seed;
	FILE*	     urandom = fopen( "/dev/urandom", "r" );
	fread( &seed, sizeof( int ), 1, urandom );
	fclose( urandom );
	srand( seed );

	disableViewUpdate( this->scapsObject );

	// Random rectangles
	for ( int i = 1; i < 20; i++ ) {
		printf( "i: %d\n", i );
		createRectangle( this->scapsObject, "test_rectangle", "", 0, 0, rand() % i, rand() % i, i );
	}

	enableViewUpdate( this->scapsObject );

	createRectangle( this->scapsObject, "test_rectangle_extra", "", 0, 0, rand() % 11, rand() % 11, 11 );

	importSVG( this->scapsObject, "C:\\scaps\\drawing.svg" );

	disableViewUpdate( this->scapsObject );
}

int
main( int argc, char** argv )
{
	static int verbose_flag;
	int	   c;

	t_scaps_struct this;

	// Setup COM
	if ( SetupCOM( &this ) == 0 ) {
		fprintf( stderr, "Setting up COM failed. Aborting.\n" );
		exit( 71 );
	}

	while ( 1 ) {
		static struct option long_options[] = { /* These options set a flag. */
							{ "verbose", no_argument, &verbose_flag, 1 },
							{ "brief", no_argument, &verbose_flag, 0 },
							/* These options don't set a flag.
							   We distinguish them by their indices. */
							{ "upload", no_argument, 0, 'u' },
							{ "download", no_argument, 0, 'd' },
							{ 0, 0, 0, 0 }
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long( argc, argv, "hud", long_options, &option_index );

		/* Detect the end of the options. */
		if ( c == -1 ) {
			break;
		}

		switch ( c ) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if ( long_options[option_index].flag != 0 ) {
					break;
				}
				printf( "option %s", long_options[option_index].name );
				if ( optarg ) {
					printf( " with arg %s", optarg );
				}
				printf( "\n" );
				break;

			case 'u':
				fprintf( stderr, "Uploading pens from STDIN\n" );
				UploadPens( &this );
				break;
			case 'd':
				fprintf( stderr, "Downloading pens to STDIN\n" );
				DownloadPens( &this );
				break;
			case 'v':
				verbose_flag = 1;
				break;
			case 'b':
				verbose_flag = 0;
				break;

			case '?':
			case 'h':
			default:
				printf( "Usage: scaps [ --upload | --download ]\n" );
		}
	}

	/* Instead of reporting ‘--verbose’
	   and ‘--brief’ as they are encountered,
	   we report the final status resulting from them. */
	if ( verbose_flag ) {
		puts( "verbose flag is set" );
	}

	/* Print any remaining command line arguments (not options). */
	if ( optind < argc ) {
		printf( "non-option ARGV-elements: " );
		while ( optind < argc ) {
			printf( "%s ", argv[optind++] );
		}
		putchar( '\n' );
	}

	ShutdownCOM( &this );

	return ( 0 );
}
