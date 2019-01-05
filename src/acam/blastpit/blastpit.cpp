// blastpit.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "blastpit.h"

#include <algorithm>
#include <vector>
#define _USE_MATH_DEFINES
#include <cstring>
#include <ctime>
#include <math.h>
#include <sstream>
#include <string>

#include "bpcommon.h"
#include "bpdrawing.h"
#include "bpelement.h"
#include "bplayer.h"
#include "bppath.h"
#include "bptext.h"
#include "pugixml.hpp"

// For sockets
#include "winsock2.h"
#include "ws2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")

// Added for AlphaCAM
#import EXE_TYPELIB_NAME implementation_only

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void NameAllToolpaths( IDrawingPtr );
bool SendPacket( struct bpPacket *packet );
UINT LoopThread( LPVOID );
int genNewID( IDrawingPtr pDrw );
int getPathID( IPathPtr path, IDrawingPtr pDrw );
int getLayerID( ILayerPtr layer, IDrawingPtr pDrw );
void setPathID( IPathPtr path, IDrawingPtr pDrw );
void setLayerID( ILayerPtr layer, IDrawingPtr pDrw );
std::string getPathName( IPathPtr path );
std::string bstr2str( _bstr_t bstr );

static bool poshelp = false;

static IAlphaCamAppPtr acamApp;

static bpDrawing bpDrw;
//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CblastpitApp

BEGIN_MESSAGE_MAP(CblastpitApp, CWinApp)
END_MESSAGE_MAP()


// CblastpitApp construction

CblastpitApp::CblastpitApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CblastpitApp object

CblastpitApp theApp;


// CblastpitApp initialization

BOOL CblastpitApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}

#define ACAMAPIFUN(type) extern "C" __declspec(dllexport) type __stdcall
#define ACAMAPP(var) IAlphaCamAppPtr acam(var_acam.pdispVal);

ACAMAPIFUN(int) InitAlphacamAddIn(VARIANT var_acam, int version)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ACAMAPP(var_acam);

	// Save a global pointer to the application,
	// for use from any function
	//acamApp = acam;

	IFramePtr frame(acam->Frame);

	static struct bpMenu {
		_bstr_t Filename;
		_bstr_t Menuname;
		_bstr_t Functionname;
	} bpMenus[] = {
		_T(""),
		_T("Set Laser Parameters"),
		_T("SetQPSet"),
		_T(""), _T(""),
		_T(""),
		_T(""), _T("Clear Laser"),
		_T("ClearLaser"),
		_T(""), _T("Output geometries"),
		_T("DataDump"),
		_T(""), _T("Save VLM"),
		_T("SaveVLM"),
		_T(""), _T("Laser Hatch"),
		_T("LaserHatch"),
		_T(""), _T("Init Laser"),
		_T(
			"LaserInit"),
		_T(""), _T("Shutdown"),
		_T(
			"LaserShutdown"),
		_T(""), _T("Test"),
		_T("LaserTest"),
	};

	for (int i = 0;
	     i < sizeof bpMenus / sizeof(struct bpMenu);
	     i++) {
		frame->AddMenuItem2(bpMenus[i].Menuname,
				    bpMenus[i].Functionname,
				    acamMenuNEW,
				    "Blastpit");

		// Check if menu item has a toolbar icon
		if (bpMenus[i].Filename != (const _bstr_t)_T("")) {
			frame->AddButton(acamButtonBarUSER1,
					 bpMenus[i].Filename,
					 frame->LastMenuCommandID);
		}
	}

	return(0);
}

// Diabled

/*
float hasHeight(std::string name)
{
	// Find a float surrounded by square brackets

	const char *start = strchr(name.c_str(), '[');
	const char *end = strchr(name.c_str(), ']');

	if (start == NULL || end == NULL) {
		return(-1);
	}
	if (end < start + 2) {
		return(-1);
	}
	end = 0;
	return((float)atof(start + 1));
}
*/

// Alphacam <-> Blastpit Helper Functions

int getLayerID(ILayerPtr layer, IDrawingPtr pDrw)
{
	int id = 0;
	_variant_t idv = layer->Attribute["ID"];

	if (idv.vt == VT_BSTR) {
		id = _wtoi(idv.bstrVal);
	}
	return(id);
}
int getPathID(IPathPtr path, IDrawingPtr pDrw)
{
	_variant_t idv = path->Attribute["ID"];
	int id = 0;

	if (idv.vt == VT_BSTR) {
		id = _wtoi(idv.bstrVal);
	}
	return(id);
}
void setLayerID(ILayerPtr layer, IDrawingPtr pDrw)
{
	int id = genNewID(pDrw);
	CString string;

	string.Format(_T("%d"), id);
	layer->Attribute["ID"] = (_variant_t)string;
}
void setPathID(IPathPtr path, IDrawingPtr pDrw)
{
	int id = genNewID(pDrw);
	CString string;

	string.Format(_T("%d"), id);
	path->Attribute["ID"] = (_variant_t)string;
}
std::string getPathName(IPathPtr path)
{
	_variant_t idv = path->Attribute["PATHNAME"];
	if (idv.vt == VT_BSTR) {
		return(bstr2str(idv.bstrVal));
	}
	return("");
}
int getHighestId(IDrawingPtr pDrw)
{
	int id, highID = 0;

	for (ILayerPtr layer = pDrw->GetFirstLayer();
		layer;
		layer = layer->GetNext()) {
		id = getLayerID(layer, pDrw))
		if ( id > highID) {
			highID = id;
		}
	}
	IPathsPtr pGeos = pDrw->Geometries;
	IPathPtr pGeo;
	for (int i = 1; i <= pGeos->Count; i++) {
		pGeo = pGeos->Item(i);
		id = getPathID(pGeo, pDrw))
		if ( id > highID) {
			highID = id;
		}
	}
	return(highID + 1);
}
int genNewID(IDrawingPtr pDrw)
{
	static int id = 0;

	if (id == 0) {
		id = getHighestId(pDrw);
	}
	return(id + 1);
}
std::string bstr2str(_bstr_t bstr)
{
	/*
	 * http://stackoverflow.com/questions/6284524/bstr-to-stdstring-stdwstring-and-vice-versa
	 */
	std::string string;
	int result = WideCharToMultiByte(CP_UTF8,
					 0,
					 bstr,
					 -1,
					 NULL,
					 0,
					 NULL,
					 NULL);
	if (result > 0) {
		string.resize(result);
		WideCharToMultiByte(CP_UTF8,
				    0,
				    bstr,
				    -1,
				    &string[0],
				    result,
				    NULL,
				    NULL);
		return(string.c_str());
	}
	return("");
}

void AddEntities(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	IAlphaCamAppPtr pApp(var_acam.pdispVal);
	IDrawingPtr pDrw(pApp->ActiveDrawing);

	// Loop through bpLayers

	// Does layer already exist?

	// Yes - update layer

	// No - Create layer
}
void DeleteEntities(void);
void UploadAllEntities(bpDatabase db)
{
/*
	// DatabaseFromDrawing: Takes the current drawing and converts it into
	// a blastpit database

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	IAlphaCamAppPtr pApp(var_acam.pdispVal);
	IDrawingPtr pDrw(pApp->ActiveDrawing);

	// Erase any old data
	bpDrw.reset();

	// Dump layers ( name, colour, linetype )
	for (ILayerPtr layer = pDrw->GetFirstLayer();
	     layer;
	     layer = layer->GetNext()) {
		std::string str = bstr2str(layer->Name);
		bpLayer blayer(str.c_str());
		int id = getLayerID(layer, pDrw);
		if (id == 0) {
			id = genNewID(pDrw);
			setLayerID(layer, pDrw);
		}
		blayer.setId(id);
		blayer.setLinetype(layer->LineType);
		blayer.setColour(layer->ColorRGB);
		bpDrw.addLayer(blayer);
	}

	// Paths
	IPathsPtr pGeos = pDrw->Geometries;
	IPathPtr pGeo;
	for (int i = 1; i <= pGeos->Count; i++) {
		pGeo = pGeos->Item(i);

		bpPath iPath;
		// Dump header
		std::string pName = getPathName(pGeo);
		if (pName.length() > 0) {
			iPath.setName(getPathName(pGeo));
		}
		int id = getPathID(pGeo, pDrw);
		if (id == 0) {
			id = genNewID(pDrw);
			setPathID(pGeo, pDrw);
		}
		iPath.setId(id);
		iPath.setColour((uint32_t)pGeo->ColorRGB);
		ILayerPtr idLayerPtr = pGeo->GetLayer();
		std::string iLayerName;
		_bstr_t abcdef = idLayerPtr->Name;
		iLayerName = bstr2str(idLayerPtr->Name);
		bpLayer *jLayer = bpDrw.getLayer(bstr2str(idLayerPtr->Name));
		iPath.setLayerId(jLayer->getId());

		// Dump elements
		IElementPtr pElem = pGeo->GetFirstElem();
		for (int element = 1;
		     element <= pGeo->Elements->Count;
		     element++) {
			if (pElem->IsLine) {
				bpElement iElement(pElem->StartXL,
						   pElem->StartYL, pElem->EndXL,
						   pElem->EndYL);
				iPath.addElement(iElement);
			}
			if (pElem->IsArc) {
				bpElement iElement(pElem->StartXL,
						   pElem->StartYL, pElem->EndXL,
						   pElem->EndYL,
						   pElem->CenterXL,
						   pElem->CenterYL,
						   pElem->CW == VARIANT_TRUE);
				iPath.addElement(iElement);
			}
			pElem = pElem->GetNext();
		}
		bpDrw.addPath(iPath);
	}

	// Text
	ITextsPtr pTexts = pDrw->Text;
	for (int i = 1;
	     i <= pTexts->Count;
	     i++) {
		ITextPtr pText = pTexts->Item(i);
		bpText iText;
		std::string fontName = pText->Font;
		if (fontName.at(0) == 'T') {
			fontName.erase(0, 1);
			iText.setFont(fontName);
		}
		iText.setHeight((float)pText->Height);
		iText.setSpacing(0.5);
		iText.setPosition(pText->LocationX, pText->LocationY);

		switch (pText->Justify) {
		case acamJustifyCENTER:
			iText.setJustification(justifyCentre);
			break;
		case acamJustifyRIGHT:
			iText.setJustification(justifyRight);
			break;
		default:
			iText.setJustification(justifyLeft);
		}

		ITextLinesPtr pTextLines = pText->Lines;
		std::stringstream ss;
		ITextLinePtr pTextLine;
		for (int j = 1; j <= pTextLines->Count; j++) {
			pTextLine = pTextLines->Item(j);
			ss << pTextLine->Text << std::endl;
		}
		std::string s = ss.str();
		iText.setText(s.c_str());
		bpDrw.addText(iText);
	}

	// Convert the database to XML and send to blastpit server
	std::stringstream stream;
	pugi::xml_document dumpXml = DatabaseToXML(db);
	dumpXml.save(stream);

	stream.seekg(0, std::ios::end);
	int streamsize = stream.tellg();
	char *buffer =
		new char[sizeof(struct bpPacket) +
			 streamsize + 1];

	bpPacket *packet = (bpPacket*)buffer;
	packet->command = bpDataDump;
	packet->id = 0;
	packet->length = streamsize;

	stream.seekg(0, std::ios::beg);
	std::string s = stream.str();
	strncpy_s((char*)&packet->data, streamsize + 1,
		  s.data(), streamsize + 1);
	SendPacket(packet);
	delete[] buffer;
*/
}
void UploadChangedEntities()
{
/* Events that we can use to monitor changes:
 *
 * AfterOpenFile
 * GeometryAdded
 * GeometriesUpdated
 * ToolpathsUpdated
 * TextAdded
 * SplineAdded
 *
 * Add a test function that checks if any entity does not have an ID.
 *
 */
}

bool Connect(SOCKET &bpSocket)
{
#define DEFAULT_BUFLEN 512

	// Setup socket
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n",
		       iResult);
		return(false);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port

	HKEY key;
	WCHAR bpServer[512];
	DWORD bpPort, size = sizeof(bpPort);
	RegOpenKeyEx(HKEY_CURRENT_USER,
		     _T(
			     "Software\\R.F. Bevan\\blastpit"), 0, KEY_READ,
		     &key);
	if (key) {
		RegQueryValueEx(key, _T(
					"bpPort"), 0, NULL,
				(LPBYTE)&bpPort,
				&size);
		DWORD bpServerSize = sizeof(bpServer);
		RegQueryValueEx(key, _T(
					"bpServer"), 0, NULL,
				(LPBYTE)bpServer,
				&bpServerSize);
		RegCloseKey(key);
	}

	CString server = (CString)bpServer;
	CString port;
	port.Format(_T("%lu"), bpPort);
	CT2A ascServer(server);
	CT2A ascPort(port);
	iResult = getaddrinfo(ascServer,
			      ascPort,
			      &hints,
			      &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n",
		       iResult);
		WSACleanup();
		return(false);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family,
				       ptr->ai_socktype,
				       ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf(
				"socket failed with error: %ld\n",
				WSAGetLastError());
			WSACleanup();
			return(false);
		}

		// Connect to server.
		iResult = connect(ConnectSocket,
				  ptr->ai_addr,
				  (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return(false);
	}

	// Start receive thread
	AfxBeginThread(ListenThread, 0);
}
UINT ListenThread(LPVOID tValue)
{
	IDrawingPtr drw(acamApp->ActiveDrawing);

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket,
			       recvbuf,
			       recvbuflen,
			       0);
		if (iResult > 0) {
			printf("Bytes received: %d\n",
			       iResult);
		} else if (iResult == 0) {
			printf("Connection closed\n");
		} else {
			printf("recv failed with error: %d\n",
			       WSAGetLastError());
		}
	} while (iResult > 0);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n",
		       WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return(false);
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return(0);                      // thread completed successfully
}

void SendCommand(int command)
{
	bpPacket packet;
	packet->command = command;
	SendPacket(&packet);
}
bool SendPacket(SOCKET bpSocket, struct bpPacket *packet)
{
#define DEFAULT_BUFLEN 512

	int iResult =
		send(bpSocket,
		     (const char*)packet,
		     (int)(sizeof(struct bpPacket) +
			   packet->length),
		     0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n",
		       WSAGetLastError());
		//closesocket(ConnectSocket);
		//WSACleanup();
		return(false);
	}

	//printf("Bytes Sent: %ld\n", iResult);
}


