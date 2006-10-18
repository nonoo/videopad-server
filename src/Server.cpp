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

    //nBindToPort = g_SettingsFile_Server.GetInt( "server settings", "udp-data-port", 62321 );
    //m_pDataSocket_UDP = new CUDPListener( szBindToIp, nBindToPort );

    memset( m_pReadBuf, 0, MAXMESSAGELENGTH+1 );
    m_nReadBufPos = 0;
}

CServer::~CServer()
{
    SAFE_DELETE( m_pControlSocket );
    SAFE_DELETE( m_pDataSocket_TCP );
    //SAFE_DELETE( m_pDataSocket_UDP );

    // freeing m_UnassignedDataConnectionVector
    for( vector<CDataConnection*>::iterator it = m_UnassignedDataConnectionVector.begin(); it != m_UnassignedDataConnectionVector.end(); it++ )
    {
	SAFE_DELETE( *it );
    }

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
}

void CServer::Loop()
{
    while( !g_fTerminate )
    {
	unsigned int nfds = 0;
	fd_set rd, wr, er;
	FD_ZERO( &rd );
	FD_ZERO( &wr );
	FD_ZERO( &er );

	// adding control socket to select
	FD_SET( m_pControlSocket->GetSocket(), &rd );
	FD_SET( m_pControlSocket->GetSocket(), &er );
	nfds = MAX( nfds, m_pControlSocket->GetSocket() );

	// adding tcp data socket to select
	FD_SET( m_pDataSocket_TCP->GetSocket(), &rd );
	FD_SET( m_pDataSocket_TCP->GetSocket(), &er );
	nfds = MAX( nfds, m_pDataSocket_TCP->GetSocket() );

	// adding client control and tcp data sockets to select
	//
	for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
	{
    	    CClient* pClient = *it;
	    // control
	    FD_SET( pClient->GetSocket(), &rd );
	    FD_SET( pClient->GetSocket(), &er );
	    nfds = MAX( nfds, pClient->GetSocket() );
	    // tcp data
	    if( pClient->GetDataConnection() != NULL )
	    {
		FD_SET( pClient->GetDataSocket(), &rd );
		FD_SET( pClient->GetDataSocket(), &er );
		nfds = MAX( nfds, pClient->GetDataSocket() );
	    }
	}

	// adding unassigned tcp data sockets to select
	for( vector<CDataConnection*>::iterator it = m_UnassignedDataConnectionVector.begin(); it != m_UnassignedDataConnectionVector.end(); it++ )
	{
	    CDataConnection* pDataConnection = *it;
	    FD_SET( pDataConnection->GetSocket(), &rd );
	    FD_SET( pDataConnection->GetSocket(), &er );
	    nfds = MAX( nfds, pDataConnection->GetSocket() );
	}



	// select blocks only for 1 second
	//
	struct timeval tOneSec;
	tOneSec.tv_sec = 1;
	tOneSec.tv_usec = 0;

	int res = select( nfds+1, &rd, &wr, &er, &tOneSec );

	// select error checking
	//
	if( ( res == -1 ) && ( errno == EINTR ) )
	{
	    continue;
	}
	if( res < 0 )
	{
	    g_Logger.Error( "select() error" );
	    break;
	}



	// new client connected to control socket
	//
	if( FD_ISSET( m_pControlSocket->GetSocket(), &rd ) )
	{
	    NewClientConnect();
	}

	// new data connection to tcp data socket
	if( FD_ISSET( m_pDataSocket_TCP->GetSocket(), &rd ) )
	{
	    NewDataConnection();
	}




	// checking unassigned data connections
	//
	for( vector<CDataConnection*>::iterator it = m_UnassignedDataConnectionVector.begin(); it != m_UnassignedDataConnectionVector.end(); it++ )
	{
	    CDataConnection* pDataConnection = *it;

	    if( FD_ISSET( pDataConnection->GetSocket(), &rd ) )
	    {
		// data is ready on this socket
		int res = ReadUnassignedDataConnection( pDataConnection );

		if( res >= 0 )
		{
		    // the connection has been assigned to a client
		    m_UnassignedDataConnectionVector.erase( it );
		    //it = m_UnassignedDataConnectionVector.begin(); // ??
		}

		// if the data connection has been dropped
		if( ( pDataConnection == NULL ) || ( res == -1 ) )
		{
		    m_UnassignedDataConnectionVector.erase( it );
		}

		if( m_UnassignedDataConnectionVector.size() == 0 )
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

	    if( FD_ISSET( pClient->GetSocket(), &rd ) )
	    {
	        // data is ready on client's control socket
	        //
	        ReadClientMessage( pClient );
	    }

	    if( pClient != NULL )
	    {
		CheckForConnectionStatus( pClient );
		CheckForLoginTimeout( pClient );

		// checking client's tcp data socket
		if( FD_ISSET( pClient->GetDataSocket(), &rd ) )
		{
		    ReadClientData( pClient->GetDataSocket(), pClient );
		}
	    }

	    // if client has been dropped
	    if( pClient == NULL )
	    {
	        if( m_ClientVector.size() == 0 )
	        {
		    break;
		}
		//it = m_ClientVector.begin(); // ??
	    }
	}
    }
}
