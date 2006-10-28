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

#include "OggStream.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

void CServer::NewDataConnection()
{
    struct sockaddr_in ClientAddr;

    // accepting new data connection
    //
    memset( &ClientAddr, 0, sizeof(ClientAddr) );
    unsigned int l = sizeof(ClientAddr);
    unsigned int nNewSocket = accept( m_pDataSocket_TCP->GetSocket(), (struct sockaddr *)&ClientAddr, &l );

    // resolving client's address
    //
    struct hostent* pNewClientHostent = NULL;
    pNewClientHostent = gethostbyaddr( &ClientAddr.sin_addr.s_addr, sizeof(ClientAddr.sin_addr.s_addr), AF_INET );

    struct in_addr inaddr;
    inaddr.s_addr = ClientAddr.sin_addr.s_addr;
    string szIP = inet_ntoa( inaddr );

    if ( nNewSocket < 0 ) {
	string tmp;

	if( pNewClientHostent != NULL )
	{
	    tmp = "can't accept client data connection from ";
	    tmp += pNewClientHostent->h_name;
	    tmp += " (" + szIP + ")";
	}
	else
	{
	    tmp = "can't accept client data connection from " + szIP;
	}

	g_Logger.Error( tmp );
	return;
    }

    // we accept data connections only from clients previously logged in
    //
    bool fKnownIP = false;
    for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	if( ( (*it)->IsLoggedin() ) && ( (*it)->GetIP() == szIP ) )
	{
	    fKnownIP = true;
	    break;
	}
    }

    if( !fKnownIP )
    {
	shutdown( nNewSocket, SHUT_RDWR );
	close( nNewSocket );
	return;
    }

    m_UnassignedDataSocketVector.push_back( nNewSocket );
}

int CServer::ReadUnassignedDataSocket( int& nSocket )
{
    int res = recv( nSocket,
	m_pOggDecoder->GetBuffer( MAXDATAREAD ), MAXDATAREAD,
	MSG_DONTWAIT );

    if( ( res <= 0 ) && ( errno != EAGAIN ) )
    {
	shutdown( nSocket, SHUT_RDWR );
	close( nSocket );
	nSocket = 0;
	return -1;
    }

    int nSerial = m_pOggDecoder->Wrote( res );
    if( nSerial >= 0 )
    {
	// ogg page is ready, we got the serial.
	// now associating socket with the client
	CClient* pClient = m_SerialMapper.GetClient( nSerial );
	if( pClient == NULL )
	{
	    return -1;
	}
	pClient->SetDataSocket( nSocket );

	// feeding ogg page into the client's associated stream
	COggStream* pOggStream = m_SerialMapper.GetOggStream( nSerial );
	if( pOggStream == NULL )
	{
	    return -1;
	}
	pOggStream->FeedPage( m_pOggDecoder->GetPage() );

	// processing page with the client
	ProcessClientData( pClient );

	return nSerial;
    }
    return -2;
}

void CServer::ReadUDPData( CUDPListener* pUDPListener )
{
    int res = recv( pUDPListener->GetSocket(),
	m_pOggDecoder->GetBuffer( MAXDATAREAD ), MAXDATAREAD,
	MSG_DONTWAIT );

    if( ( res <= 0 ) && ( errno != EAGAIN ) )
    {
	return;
    }

    int nSerial = m_pOggDecoder->Wrote( res );
    if( nSerial >= 0 )
    {
	// ogg page is ready, we got the serial.
	CClient* pClient = m_SerialMapper.GetClient( nSerial );

	// getting the associated ogg stream for the page we want to process
	COggStream* pOggStream = m_SerialMapper.GetOggStream(
	    m_pOggDecoder->GetCurrentPageSerial() );

	if( pOggStream != NULL )
	{
	    pOggStream->FeedPage( m_pOggDecoder->GetPage() );
	}

	// processing page with the client
	ProcessClientData( pClient );
    }
}

void CServer::ReadClientDataSocket( CClient*& pClient )
{
    int res = recv( pClient->GetDataSocket(),
	m_pOggDecoder->GetBuffer( MAXDATAREAD ),
	MAXDATAREAD, MSG_DONTWAIT );

    if( ( res <= 0 ) && ( errno != EAGAIN ) )
    {
	DeleteClient( pClient );
	return;
    }

    int nSerial = m_pOggDecoder->Wrote( res );
    if( nSerial >= 0 )
    {
	// getting the associated ogg stream for the page we want to process
	COggStream* pOggStream = m_SerialMapper.GetOggStream(
	    m_pOggDecoder->GetCurrentPageSerial() );

	pOggStream->FeedPage( m_pOggDecoder->GetPage() );

	// processing page with the client
	ProcessClientData( pClient );
    }
}

void CServer::ProcessClientData( CClient*& pClient )
{
    if( pClient == NULL )
    {
	return;
    } 

    COggStream* pOggStream = m_SerialMapper.GetOggStream(
        m_pOggDecoder->GetCurrentPageSerial() );

    if( pOggStream == NULL )
    {
	return;
    }

    // sending incoming ogg page to all the channels the sender client is on
    //
    for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
    {
        CChannel* pChannel = it->second;
        if( pChannel->IsOn( pClient ) )
        {
	    char* pData = pOggStream->GetRawPage();
	    unsigned int nDataSize = pOggStream->GetRawPageSize();
	    pChannel->BroadcastData( pData, nDataSize, pClient );
	    SAFE_DELETE_ARRAY( pData );
	}
    }
}
