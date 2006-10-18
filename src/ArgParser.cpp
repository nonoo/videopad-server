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

#include <iostream>

void CArgParser::PrintWelcome()
{
    cout << PACKAGE << " v" << VERSION << endl;
    cout << "URL: http://www.nonoo.hu/videopad/" << endl;

    if( DEBUG )
    {
	cout << "Debugging mode ON" << endl;
    }

    cout << endl;
}

void CArgParser::PrintHelp()
{
    cout << "Available command-line switches:" << endl << endl;
    cout << "     --help (-h)                  -  this help" << endl;
    cout << "     --config-path (-cp) [path]   -  path to config files" << endl;
    cout << "     --daemon (-d)                -  fork to background" << endl << endl;
}

CArgParser::CArgParser( int argc, char* argv[] )
{
    int i;

    // parsing other arguments
    //
    for( i=1; i < argc; i++ )
    {
	string sArg = argv[i];

	if( ( sArg == "--daemon" ) || ( sArg == "-d" ) )
	{
	    g_fDaemonMode = true;
	}

	if( ( sArg == "--help" ) || ( sArg == "-h" ) )
	{
	    PrintWelcome();
	    PrintHelp();
	    exit( EXIT_SUCCESS );
	}

	if( ( sArg == "--config-path" ) || ( sArg == "-cp" ) )
	{
	    i++;
	    sArg = argv[i];

	    g_SettingsFile_Server.SetConfigPath( sArg );
	    continue;
	}
    }
}
