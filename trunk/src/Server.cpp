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
#include "Server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <cerrno>

CServer::CServer()
{
    // tcp control socket init
    //
    string szBindToIp = g_SettingsFile_Server.Get( "server settings", "bind-to-ip", "0.0.0.0" );
    int nBindToPort = g_SettingsFile_Server.GetInt( "server settings", "tcp-control-port", 62320 );
    m_pControlSocket = new CTCPListener( szBindToIp, nBindToPort );

    nBindToPort = g_SettingsFile_Server.GetInt( "server settings", "tcp-data-port", 62321 );
    m_pDataSocket_TCP = new CTCPListener( szBindToIp, nBindToPort );

    nBindToPort = g_SettingsFile_Server.GetInt( "server settings", "udp-data-port", 62321 );
    m_pDataSocket_UDP = new CUDPListener( szBindToIp, nBindToPort );

    m_pOggDecoder = new COggDecoder;
}

CServer::~CServer()
{
    SAFE_DELETE( m_pControlSocket );
    SAFE_DELETE( m_pDataSocket_TCP );
    SAFE_DELETE( m_pDataSocket_UDP );

    // freeing m_ClientVector
    for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	SAFE_DELETE( *it );
    }

    // freeing m_ChannelMap
    for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
    {
	SAFE_DELETE( it->second );
    }

    SAFE_DELETE( m_pOggDecoder );
}

void CServer::Loop()
{
    while( !g_fTerminate )
    {
	int nfds = 0;
	fd_set rd, er;
	FD_ZERO( &rd );
	FD_ZERO( &er );

	// adding control listener socket to select
	FD_SET( m_pControlSocket->GetSocket(), &rd );
	FD_SET( m_pControlSocket->GetSocket(), &er );
	nfds = MAX( nfds, m_pControlSocket->GetSocket() );

	// adding tcp data listener socket to select
	FD_SET( m_pDataSocket_TCP->GetSocket(), &rd );
	FD_SET( m_pDataSocket_TCP->GetSocket(), &er );
	nfds = MAX( nfds, m_pDataSocket_TCP->GetSocket() );

	// adding udp data socket to select
	FD_SET( m_pDataSocket_UDP->GetSocket(), &rd );
	FD_SET( m_pDataSocket_UDP->GetSocket(), &er );
	nfds = MAX( nfds, m_pDataSocket_UDP->GetSocket() );

	// adding client control and tcp data sockets to select
	//
	for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
	{
    	    CClient* pClient = *it;

	    // control
	    FD_SET( pClient->GetControlSocket(), &rd );
	    FD_SET( pClient->GetControlSocket(), &er );
	    nfds = MAX( nfds, pClient->GetControlSocket() );
	    // tcp data
	    if( pClient->GetDataSocket() >= 0 )
	    {
		FD_SET( pClient->GetDataSocket(), &rd );
		FD_SET( pClient->GetDataSocket(), &er );
		nfds = MAX( nfds, pClient->GetDataSocket() );
	    }
	}

	// adding unassigned tcp data sockets to select
	for( vector<int>::iterator it = m_UnassignedDataSocketVector.begin(); it != m_UnassignedDataSocketVector.end(); it++ )
	{
	    FD_SET( *it, &rd );
	    FD_SET( *it, &er );
	    nfds = MAX( nfds, *it );
	}



	// select blocks only for 1 second
	//
	struct timeval tOneSec;
	tOneSec.tv_sec = 1;
	tOneSec.tv_usec = 0;

	int res = select( nfds+1, &rd, NULL, &er, &tOneSec );

	// select error checking
	//
	if( ( res == -1 ) && ( errno == EINTR ) )
	{
	    continue;
	}
	if( res < 0 )
	{
	    if( errno == ENOMEM )
	    {
		g_Logger.Error( "select() error: not enough memory" );
		break;
	    }
	    continue;
	}





	// new client connected to control socket
	//
	if( ( FD_ISSET( m_pControlSocket->GetSocket(), &rd ) ) &&
	    ( !FD_ISSET( m_pControlSocket->GetSocket(), &er ) ) )
	{
	    NewClientConnect();
	}

	// new data connection to tcp data socket
	//
	if( ( FD_ISSET( m_pDataSocket_TCP->GetSocket(), &rd ) ) &&
	    ( !FD_ISSET( m_pDataSocket_TCP->GetSocket(), &er ) ) )
	{
	    NewDataConnection();
	}





	// checking unassigned data connections
	//
	for( vector<int>::iterator it = m_UnassignedDataSocketVector.begin(); it != m_UnassignedDataSocketVector.end(); it++ )
	{
	    if( ( FD_ISSET( *it, &rd ) ) &&
		( !FD_ISSET( *it, &er ) ) )
	    {
		// data is ready on this socket
		int res = ReadUnassignedDataSocket( *it );

		if( res >= 0 )
		{
		    // the socket has been assigned to a client
		    m_UnassignedDataSocketVector.erase( it );
		    FD_CLR( *it, &rd );
		    //it = m_UnassignedDataSocketVector.begin(); // ??
		}

		// if the data connection has been dropped
		if( ( *it == -1 ) || ( res == -1 ) )
		{
		    shutdown( *it, SHUT_RDWR );
		    close( *it );
		    m_UnassignedDataSocketVector.erase( it );
		}

		if( m_UnassignedDataSocketVector.size() == 0 )
		{
		    break;
		}
	    }

	    // checking for socket errors
	    //
	    if( FD_ISSET( *it, &er ) )
	    {
		m_UnassignedDataSocketVector.erase( it );
		if( m_UnassignedDataSocketVector.size() == 0 )
		{
		    break;
		}
	    }
	}




	// checking client control and data connections
	//
	for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
	{
    	    CClient* pClient = *it;

	    // checking for socket errors
	    //
	    if( FD_ISSET( pClient->GetControlSocket(), &er ) )
	    {
		pClient->SetQuitMessage( "Connection reset by peer" );
		DeleteClient( pClient );
	    }

	    // checking for data socket errors
	    //
	    if( pClient != NULL )
	    {
		// if the client has a data connection
		//
		if( pClient->GetDataSocket() >= 0 )
		{
		    if( FD_ISSET( pClient->GetDataSocket(), &er ) )
		    {
			pClient->SetQuitMessage( "Data connection reset by peer" );
			DeleteClient( pClient );
		    }
		}
	    }

	    if( pClient != NULL )
	    {
		if( FD_ISSET( pClient->GetControlSocket(), &rd ) )
		{
	    	    // data is ready on client's control socket
	    	    //
	    	    ReadClientMessage( pClient );
		}
	    }

	    // if client hasn't been dropped
	    //
	    if( pClient != NULL )
	    {
		CheckForConnectionStatus( pClient );
		CheckForLoginTimeout( pClient );

	    }

	    if( pClient != NULL )
	    {
	        // checking client's tcp data socket if it is valid
		//
	        if( pClient->GetDataSocket() >= 0 )
	        {
		    if( FD_ISSET( pClient->GetDataSocket(), &rd ) )
		    {
		        ReadClientDataSocket( pClient );
		    }
		}
	    }

	    // if client has been dropped
	    //
	    if( pClient == NULL )
	    {
	        if( m_ClientVector.size() == 0 )
	        {
		    break;
		}
		it = m_ClientVector.begin();
	    }
	}




	// data is ready on udp data socket
	//
        if( FD_ISSET( m_pDataSocket_UDP->GetSocket(), &rd ) )
        {
	    ReadUDPData( m_pDataSocket_UDP );
	}
    }
}
