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
#include "Logger.h"

#include <syslog.h>
#include <iostream>

void CLogger::Info( string& szString )
{
    Info( szString.c_str() );
}

void CLogger::Info( const char* pString )
{
    if ( g_fDaemonMode )
    {
	syslog(LOG_INFO, "%s", pString );
    }
    else
    {
	cout << pString << endl;
    }
}

void CLogger::Error( string& szString )
{
    Error( szString.c_str() );
}

void CLogger::Error( const char* pString )
{
    if ( g_fDaemonMode )
    {
	syslog(LOG_ERR, "%s", pString );
    }
    else
    {
	cerr << pString << endl;
    }
}

CLogger::CLogger()
{
    // opening system logger
    openlog( PACKAGE, LOG_PID, LOG_USER );
}

CLogger::~CLogger()
{
    closelog();
}
