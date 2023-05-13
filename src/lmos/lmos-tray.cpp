/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
/****************************************************************************
 *
 * Copyright (c) 2014 Andrew Black
 *
 * Permission is  hereby granted, free of  charge, to any person  obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software  without restriction, including without limitation
 * the rights to  use, copy, modify, merge,  publish, distribute, sublicense,
 * and/or sell  copies of  the Software,  and to permit  persons to  whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT  NOT LIMITED TO THE  WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR  A PARTICULAR PURPOSE  AND NONINFRINGEMENT. IN NO  EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER  IN AN ACTION  OF CONTRACT, TORT OR  OTHERWISE, ARISING
 * FROM,  OUT OF  OR IN  CONNECTION WITH  THE SOFTWARE  OR THE  USE OR  OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include "lmos-tray.hpp"
#include "ui_lmos-tray.h"

#include "parser.hpp"
#include "traysettings.h"
#include <QMenu>
#include <QMessageBox>
#include <QtGui>
#include <sstream>

LmosTray::LmosTray( QWidget* parent )
	: QMainWindow( parent )
	, ui( new Ui::MainWindow )
{
	ui->setupUi( this );

	createActions();
	initTray();
	connect( trayIcon,
		 SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
		 this,
		 SLOT( iconActivated( QSystemTrayIcon::ActivationReason ) ) );
	setIcon( "noconnection" );
	trayIcon->show();

#ifdef VER
	setWindowTitle( "Lmos [git] " + QString( VER ) );
#endif
	listen();

	// #ifdef DEBUG
	// Always show the log window when debugging
	show();
	// #endif
}

LmosTray::~LmosTray()
{
	// destroyLMOS();
	if ( parser ) {
		delete parser;
	}
	delete ui;
}

void
LmosTray::createActions()
{
	minimizeAction = new QAction( tr( "Mi&nimize" ), this );
	connect( minimizeAction, SIGNAL( triggered() ), this, SLOT( hide() ) );

	restoreAction = new QAction( tr( "&Restore" ), this );
	connect( restoreAction, SIGNAL( triggered() ), this, SLOT( showNormal() ) );

	quitAction = new QAction( tr( "&Quit" ), this );
	connect( quitAction, SIGNAL( triggered() ), qApp, SLOT( quit() ) );

	settingsAction = new QAction( tr( "&Settings" ), this );
	connect( settingsAction, SIGNAL( triggered() ), this, SLOT( showSettings() ) );

	clearLogAction = new QAction( tr( "&Clear log" ), this );
	connect( clearLogAction, SIGNAL( triggered() ), this, SLOT( clearLog() ) );
}
void
LmosTray::initTray()
{
	trayIconMenu = new QMenu( this );
	// QAction* clearLogAction = new QAction( "&Clear log", this );
	trayIconMenu->addAction( clearLogAction );
	trayIconMenu->addSeparator();
	trayIconMenu->addAction( minimizeAction );
	trayIconMenu->addAction( restoreAction );
	trayIconMenu->addSeparator();
	trayIconMenu->addAction( settingsAction );
	trayIconMenu->addAction( quitAction );

	trayIcon = new QSystemTrayIcon( this );
	trayIcon->setContextMenu( trayIconMenu );
}

void
LmosTray::showSettings()
{
	/* Show the settings window */
	traysettings* ts = new traysettings;
	if ( parser ) {
		connect( ts, SIGNAL( serverChanged() ), parser, SLOT( wsConnect() ) );
	}
	ts->setAttribute( Qt::WA_DeleteOnClose );
	ts->show();
}

void
LmosTray::setIcon( QString png )
{
	QIcon icon( ":///" + png + ".png" );
	// QIcon icon(":///lmos-tray.png");

	trayIcon->setIcon( icon );
	setWindowIcon( icon );
}
void
LmosTray::closeEvent( QCloseEvent* event )
{
	if ( trayIcon->isVisible() ) {
		hide();
		event->ignore();
	}
}
void
LmosTray::setTrayBalloon( QString text )
{
	trayIcon->showMessage( "Lmos", text );
}
void
LmosTray::iconActivated( QSystemTrayIcon::ActivationReason reason )
{
	switch ( reason ) {
		case QSystemTrayIcon::Trigger:
			isVisible() ? hide() : show();
			break;
		case QSystemTrayIcon::DoubleClick:
			break;
		case QSystemTrayIcon::MiddleClick:
			break;
		default:;
	}
}

void
LmosTray::listen()
{
	parser = new Parser( nullptr );
	connect( parser, SIGNAL( finished() ), qApp, SLOT( quit() ) );
	connect( parser, SIGNAL( sendlog( QString ) ), this, SLOT( log( QString ) ) );
	connect( parser, SIGNAL( settray( QString ) ), this, SLOT( setTrayBalloon( QString ) ) );
	connect( parser, SIGNAL( seticon( QString ) ), this, SLOT( setIcon( QString ) ) );
	connect( parser, SIGNAL( ClearLog() ), this, SLOT( clearLog() ) );
}

void
LmosTray::clearLog()
{
	ui->teLog->clear();
}

void
LmosTray::log( QString entry )
{
	ui->teLog->append( entry );
}

void
LmosTray::alert( const QString& name, int argc, void* argv )
{
	(void)argc;
	(void)argv;

	ui->teLog->append( "Alert: " + name );
}

void
LmosTray::log( int level, const char* function, QString entry )
{
	QString log	= QString::number( level );
	QString logFunc = QString( function );
	ui->teLog->append( log + ": (" + logFunc + ") " + entry );
}

void
LmosTray::retval( const char* function, bool value )
{
	QString logFunc = QString( function );
	QString retbool = value ? "True" : "False";
	ui->teLog->append( "retval (" + logFunc + "): " + retbool );
}

void
LmosTray::retval( const char* function, int value )
{
	QString logFunc = QString( function );
	ui->teLog->append( "retval (" + logFunc + "): " + QString::number( value ) );
}
