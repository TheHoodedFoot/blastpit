#include "traysettings.h"
#include "ui_traysettings.h"
#include <QSettings>

traysettings::traysettings( QWidget* parent )
	: QDialog( parent )
	, ui( new Ui::traysettings )
{
	ui->setupUi( this );

	QSettings traySettings( "Rfbevanco", "lmos-tray" );

	/* Retrieve settings */
	QString dummy = traySettings.value( "wsServer" ).toString();
	if ( dummy.trimmed().isEmpty() ) {
		dummy = "ws://server:port";
	}
	ui->wsServer->setText( dummy );

	connect( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( storesettings() ) );
}

traysettings::~traysettings()
{
	delete ui;
}

void traysettings::storesettings()
{
	QSettings traySettings( "Rfbevanco", "lmos-tray" );

	/* Store settings */
	traySettings.setValue( "wsServer", ui->wsServer->text() );
	emit( serverChanged() );
}
