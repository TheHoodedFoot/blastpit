#ifndef LIBLMOS_H
#define LIBLMOS_H

#include <QObject>
#include <QPixmap>
#include <QtCore>

#if defined( Q_OS_WIN32 )
#include "lmosactxlib.h"
using namespace LMOSACTXLib;
#endif


class Lmos : public QObject
{
	Q_OBJECT

      public:
	explicit Lmos( QObject* parent = nullptr );
	~Lmos();

      signals:
	void log( QString );
	void ack( QString );
	void log( int, const char*, QString );
	void sigMouseDown();
	void sendEvent( int, QString );
	void sigImageEnd2( double, int );
#if defined( Q_OS_WIN32 )
	void retval( const char*, QVariant );
#endif
	void retval( const char*, bool );
	void retval( const char*, int );

      public slots:
#if defined( Q_OS_WIN32 )
	void imageEnd2( double, ImageResultConstants );
#endif
	void alarm( int alarmNum, QString description, QString moName, int moID );
	void currentChanged( double );
	void emitSig( const QString& );
	void exception( int, const QString&, const QString&, const QString& );
	void frequencyChanged( int );
	void imagebegin();
	void imageend();
	void jobBegin();
	void jobEnd();
	void logstring( const QString& name, int argc, void* argv );
	void messagemap( QString& name );
	void mOBeginName( QString& );
	void mOEndName( QString& );
	/* void mouseDown(int, int, double, double); */
	void plcEvent( QString&, QString&, QString& );

      public:
	void	AddQPSet( const QString&, double, int, int );
	int	CancelJob();
	void	ClearQPSets();
	void	ConnectSignals();
	void	CreateControl();
	void	DestroyControl();
	void	DisconnectSignals();
	void	ForceRedraw();
	double	GetW();
	QPixmap GrabWindow();
	void	HideWindow();
	void	ImportXML();
	void	ImportXML( QTemporaryFile* );
	bool	InitMachine();
	void	LayerSetExportable( const QString&, const bool );
	void	LayerSetHeightZAxis( const QString&, float );
	void	LayerSetLaserable( const QString&, const bool );
	void	LayerSetVisible( const QString&, const bool );
	bool	LoadJob();
	bool	LoadVLM( const QString& );
	bool	LoadXML( const QString& );
	void	MoveW( double );
	void	MoveZ( float );
	int	ReadByte( const int, const int );
	int	ReadIOBit( const QString& );
	void	Reference();
	void	SaveQPSets();
	bool	SaveVLM( const QString& );
	int	PatchFlexibleShadows( const QString& );
	int	PatchVLMFile( const QString&	   filename,
			      int		   length,
			      const unsigned char* findstring,
			      const unsigned char* replacestring );
	void	SetDimension( const QString&, const double, const double );
	void	SetLaserable( QString object, int laserable );
	void	SetMOLayer( const QString&, const QString& );
	void	SetPosValues( const QString&, const double, const double, const double );
	void	SetQualityParam( const QString&, const int, const int, const QVariant&, const bool );
	void	ShowMarkingArea();
	void	ShowWindow();
	bool	StartMarking();
	void	StartPosHelp( const QString& );
	void	StopMarking();
	void	StopPosHelp();
	void	SuppressRedraw( int redraw );
	void	TermMachine();
	void	Test();
	QString GetGeoList();
	void	UnsetLaserableAllObjects();
	void	WriteByte( const int, const char, const char );
	void	WriteIOBit( const QString&, const bool );
	void	ZoomWindow( int, int, int, int );


      private:
	bool Call( char const* ) const;
	void sendIdEvent( QString string, int id );

#if defined( Q_OS_WIN32 )
	LMOSACTXLib::LMOSActX* lmos_actx;
#endif
};
#endif	// LIBLMOS_H
