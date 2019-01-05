#include "blastpit.h"
#include "layer.h"
#include "network.h"
#include "path.h"
#include "pugixml.hpp"
#include "segment.h"
#include "text.h"

Blastpit::Blastpit()
{
	xml = new (pugi::xml_document);
	network = nullptr;
}

/// Creates a new drawing from blastpit-compatible XML.
Blastpit::Blastpit(pugi::xml_document *bp_xml)
{
	xml = new (pugi::xml_document);
	network = nullptr;

	(void)bp_xml;
}
Blastpit::~Blastpit()
{
	if (network != nullptr) delete network;
	delete xml;
}

Layer *
Blastpit::AddLayer(Layer *layer)
{
	layers.push_back(layer);
	return *--layers.end();
}
void
Blastpit::RemoveLayer(Layer *layer)
{
	(void)layer;
	// layers.erase(layer);
}
Path *
Blastpit::AddPath(Path *path)
{
	paths.push_back(path);
	return *--paths.end();
}
void
Blastpit::RemovePath(Path *path)
{
	(void)path;

	// paths.erase(path);
}
Text *
Blastpit::AddText(Text *text)
{
	texts.push_back(text);
	return *--texts.end();
}
void
Blastpit::RemoveText(Text *text)
{
	(void)text;
	// texts.erase(text);
}
int
Blastpit::GetLayerCount()
{
	return layers.size();
}
int
Blastpit::GetPathCount()
{
	return paths.size();
}
int
Blastpit::GetTextCount()
{
	return texts.size();
}
Layer *
Blastpit::GetLayerByID(int id)
{
	for (std::list<Layer *>::iterator it = layers.begin();
	     it != layers.end(); ++it) {
		if ((*it)->GetID() == id) {
			return *it;
		}
	}
	return NULL;
}
Layer *
Blastpit::GetLayerByName(std::string name)
{
	for (std::list<Layer *>::iterator it = layers.begin();
	     it != layers.end(); ++it) {
		if (name == (*it)->GetName()) {
			return *it;
		}
	}
	return 0;
}
Path *
Blastpit::GetPathByID(int id)
{
	for (std::list<Path *>::iterator it = paths.begin();
	     it != paths.end(); ++it) {
		if ((*it)->GetID() == id) {
			return *it;
		}
	}
	return 0;
}
Text *
Blastpit::GetTextByID(int id)
{
	for (std::list<Text *>::iterator it = texts.begin();
	     it != texts.end(); ++it) {
		if ((*it)->GetID() == id) {
			return *it;
		}
	}
	return 0;
}

/// Get the drawing as blastpit XML.
/// \return Pointer to XML document
pugi::xml_document *
Blastpit::GetBpXML()
{
	return NULL;
}
/// Get the drawing as Rofin-compatible XML.
/// Note that this discards any
/// information that is incompatible with the Rofin importer.
/// \return Pointer to Rofin-compatible XML document
pugi::xml_document *
Blastpit::GetRofinXML()
{
	return NULL;
}
/// Connect to a blastpit TCP server.
/// Creates an instance of a Network class and attempts to connect.
/// If successful it stores as the network member variable.
/// On failure it sets the network member variable to NULL.
/// \param [in] IP_Address An IP address or resolvable network address
/// \param [in] port The TCP port number
int
Blastpit::Connect(std::string IP_Address, int port)
{
	(void)IP_Address;
	(void)port;

	network = NULL;
	return false;
}

/// Connect to a blastpit UDP socket.
/// Creates an instance of a Network class and attempts to connect.
/// If successful it stores as the network member variable.
/// On failure it sets the network member variable to NULL.
/// \param [in] IP_Address An IP address or resolvable network address
void
Blastpit::Connect(std::string IP_Address)
{
	(void)IP_Address;

	network = NULL;
}
/// Disconnect the TCP or UDP connection.
/// Disconnects the network connection and deletes the network object.
void
Blastpit::Disconnect()
{
	if (network != NULL) delete network;
}

/// Sends a simple command over the network.
/// Sends a command without a data packet to a TCP or UDP socket.
/// \param [in] command A BpCommand that requires no additional data.
/// \exception NetWork Error
void
Blastpit::SendCommand(int command)
{
	(void)command;
}
/// Sends a standard command over the network.
/// Sends a command with a data packet to a TCP or UDP socket.
/// \param [in] command A BpCommand
/// \param [in] data A string containing the data packet.
/// \exception NetWork Error
void
Blastpit::SendPacket(int command, std::string data)
{
	(void)command;
	(void)data;
}
std::string
Blastpit::ReceivePacket()
{
	return "error";
}
/// Checks if a valid connection to the server exists
/// \return 0 if connection is OK, otherwise a network error code
int
Blastpit::IsConnected()
{
	return false;
}
int
Blastpit::Burn()
{
	return false;
}
int
Blastpit::OpenDoor()
{
	return false;
}
/// Retrieves the live reported height of the laser table.
/// \return -1 if error, otherwise height in mm (0 - 120)
float
Blastpit::GetLiveHeight()
{
	return -1;
}
// Perhaps have a bp.Status(), which returns a structure providing
// info about the current status of the laser. Also, bp.Wait(msec), which
// polls the status and returns when there are no running tasks, or times out.
