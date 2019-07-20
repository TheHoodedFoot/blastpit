#ifndef BLASTPIT
#define BLASTPIT

#include <list>
#include <memory>
#include <string>

namespace pugi {
class xml_document;
}
/* class Path; */
/* class Layer; */
/* class Text; */
class Network;

class Blastpit {
      public:
	Blastpit();
	Blastpit(pugi::xml_document *bp_xml);
	virtual ~Blastpit();

	/* Layer *AddLayer(Layer *layer); */
	/* Path *AddPath(Path *path); */
	/* Text *AddText(Text *text); */

	/* void RemoveLayer(Layer *layer); */
	/* void RemoveLayer(int id); */
	/* void RemovePath(Path *path); */
	/* void RemovePath(int id); */
	/* void RemoveText(Text *text); */
	/* void RemoveText(int id); */

	/* int GetLayerCount(); */
	/* int GetPathCount(); */
	/* int GetTextCount(); */

	/* Layer *GetLayerByName(std::string name); */
	/* Layer *GetLayerByID(int id); */
	/* Path *GetPathByID(int id); */
	/* Text *GetTextByID(int id); */

	pugi::xml_document *GetBpXML();
	pugi::xml_document *GetRofinXML();

	int Connect(std::string IP_Address, int port);
	void Connect(std::string IP_Address);
	void Disconnect();
	int IsConnected();
	void SendCommand(int command);
	void SendPacket(int command, std::string data);
	std::string ReceivePacket();
	int Burn();
	int OpenDoor();
	float GetLiveHeight();

      private:
	/* std::list<Layer *> layers; */
	/* std::list<Path *> paths; */
	/* std::list<Text *> texts; */
	Network *network;  ///< Pointer to network instance

	// std::unique_ptr<Network> netClient(nullptr);
	// std::unique_ptr<Parser> netServer(nullptr);

	pugi::xml_document *xml;
};
#endif /* end of include guard: BLASTPIT */
