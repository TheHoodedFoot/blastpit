

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 8.00.0603 */
/* at Tue Aug 02 08:55:31 2022
 */
/* Compiler settings for .\ctrl_projects\samlight_client\sc_samlight_client_ctrl_ex_type_lib.idl:
    Oicf, W1, Zp1, env=Win32 (32b run), target_arch=X86 8.00.0603
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data
    VC __declspec() decoration level:
	 __declspec(uuid()), __declspec(selectany), __declspec(novtable)
	 DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable : 4049 ) /* more than 64k source lines */


#ifdef __cplusplus
extern "C"
{
#endif


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID( type, name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 )                                      \
	DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 )

#else  // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
	unsigned long  x;
	unsigned short s1;
	unsigned short s2;
	unsigned char  c[8];
} IID;

#endif	// __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif	// CLSID_DEFINED

#define MIDL_DEFINE_GUID( type, name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 )                                      \
	const type name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

#endif !_MIDL_USE_GUIDDEF_

	MIDL_DEFINE_GUID( IID,
			  LIBID_SAMLIGHT_CLIENT_CTRL_EXLib,
			  0xA9741D9D,
			  0xE7CB,
			  0x4ff3,
			  0x82,
			  0x68,
			  0x0C,
			  0x3B,
			  0x55,
			  0xEC,
			  0x4B,
			  0x52 );


	MIDL_DEFINE_GUID( IID,
			  IID__Samlight_clientEx,
			  0xD81CCA6E,
			  0x4FEA,
			  0x49b3,
			  0xA8,
			  0x2E,
			  0x32,
			  0x13,
			  0x7F,
			  0x10,
			  0x17,
			  0x28 );


	MIDL_DEFINE_GUID( IID,
			  DIID__DSamlight_clientEx,
			  0xA47068AD,
			  0xAF94,
			  0x4E91,
			  0x84,
			  0x52,
			  0x46,
			  0x92,
			  0x34,
			  0xE5,
			  0x98,
			  0xD3 );


	MIDL_DEFINE_GUID( CLSID,
			  CLSID_ScSamlightClientCtrlEx,
			  0x3E6FBC49,
			  0x6C3D,
			  0x4c10,
			  0x81,
			  0xDD,
			  0xDA,
			  0xD7,
			  0x4E,
			  0x7B,
			  0x39,
			  0xAC );

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif
