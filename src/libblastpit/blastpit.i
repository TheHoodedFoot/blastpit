%module blastpit
%include "std_string.i"
%{
#include <blastpit.h>
#include <layer.h>
#include <path.h>
#include <text.h>
%}

class Blastpit {

public:
	Blastpit();

        Layer *AddLayer( Layer *layer );
        Path *AddPath( Path *path );
        Text *AddText( Text *text );
	int GetLayerCount();
	int GetPathCount();
	int GetTextCount();

        void Connect( std::string IP_Address, int port );
        void Connect( std::string IP_Address );
        void Disconnect();
        void SendCommand( int command );
        void SendPacket( int command, std::string data );
        std::string ReceivePacket ();
};

class Layer {

public:
        Layer( std::string layer_name );
        Layer( std::string layer_name, float height, unsigned int colour );
        Layer( std::string layer_name, float height, unsigned int colour,
                unsigned int linetype, bool is_laserable );

        std::string GetName();
        float GetHeight();
        unsigned int GetColour();
        unsigned int GetLinetype();
        bool GetIsLaserable();
        int GetID();

        void SetName( std::string layer_name );
        void SetColour( unsigned int colour );
        void SetLinetype( unsigned int linetype );
        void SetHeight( float height );
        void SetIsLaserable( bool is_laserable );

        int AddPath( Path *path );
};

class Path {

public:
	Path();

        int Circle( double x, double y, double radius );
        int Rectangle( double x1, double y1, double x2, double y2 );
        int Line( double x1, double y1, double x2, double y2 );
        void Rotate( double x, double y, double angle );

        int SetHatchType( std::string hatch );
        int SetMarkingType( std::string type );
};

class Text {

public:
	Text();
};

