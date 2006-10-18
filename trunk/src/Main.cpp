//  This file is part of videopad-server.
//    
//  videopad-server is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  videopad-server is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with videopad-server; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "Main.h"
#include "ArgParser.h"
#include "Server.h"

#include <sys/stat.h>
#include <csignal>

CSettingsFile g_SettingsFile_Server;
CLogger g_Logger;
bool g_fDaemonMode = false;
bool g_fTerminate = false;

void Daemonize()
{
    int res = fork();
    if( res < 0 )
    {
	g_fDaemonMode = false;
	g_Logger.Error( "can't fork into the background!" );
	return;
    }
    if( res > 0 )
    {
	// parent exiting
	//
	exit( EXIT_SUCCESS );
    }

    // become a process group leader
    setsid();

    // ignoring SIGCHLD
    signal( SIGCHLD, SIG_IGN );

    chdir( "/" );
}

// SIGHUP signal handler
void OnSIGHUP( int )
{
    // reloading main config file
    g_SettingsFile_Server.LoadConfig();
}

void OnSIGTERM( int )
{
    g_fTerminate = true;
}

void AtExit()
{
    g_Logger.Info( "exiting." );
}

int main( int argc, char* argv[] )
{
    umask( 177 ); // chmod 600
    signal( SIGTERM, OnSIGTERM );
    signal( SIGINT, OnSIGTERM );

    CArgParser* ArgParser = new CArgParser( argc, argv );
    SAFE_DELETE( ArgParser );

    atexit( AtExit );

    // main config file init
    //
    string szMainConfName = PACKAGE;
    szMainConfName += ".conf";
    g_SettingsFile_Server.SetConfigFile( szMainConfName );
    g_SettingsFile_Server.LoadConfig();

    if( g_fDaemonMode )
    {
	Daemonize();
    }

    signal( SIGHUP, OnSIGHUP );
    g_Logger.Info( "starting server" );

    CServer* Server = new CServer;
    Server->Loop();
    SAFE_DELETE( Server );
}
