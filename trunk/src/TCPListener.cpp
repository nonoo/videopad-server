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
#include "TCPListener.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>

CTCPListener::CTCPListener( string& szBindIP, int nPort )
{
    if ( ( m_Socket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	string tmp = "can't create TCP socket on port ";
	tmp += nPort;
	g_Logger.Error( tmp );
	exit( -1 );
    }

    int yes=1;
    if ( setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof(yes) ) < 0 )
    {
	string tmp = "can't create TCP socket on port ";
	tmp += nPort;
	g_Logger.Error( tmp );
	exit( -1 );
    }

    struct sockaddr_in servaddr;
    memset( &servaddr, 0, sizeof(servaddr) );

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( nPort );

    struct hostent *hp = gethostbyname( szBindIP.c_str() );

    if( hp == NULL )
    {
	string tmp = "can't bind to " + szBindIP;
	tmp += ":";
	tmp += nPort;
	g_Logger.Error( tmp );
	exit( -1 );
    }

    memcpy( &servaddr.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length );

    if ( bind( m_Socket, (struct sockaddr *)&servaddr, sizeof(servaddr) ) < 0 )
    {
	string tmp = "can't bind to " + szBindIP;
	tmp += ":";
	tmp += nPort;
	g_Logger.Error( tmp );
	exit( -1 );
    }

    if( listen( m_Socket, SOMAXCONN ) < 0 )
    {
	string tmp = "can't listen on " + szBindIP;
	tmp += ":";
	tmp += nPort;
	g_Logger.Error( tmp );
	exit( -1 );
    }
}

CTCPListener::~CTCPListener()
{
    close( m_Socket );
}

const unsigned int& CTCPListener::GetSocket()
{
    return m_Socket;
}
