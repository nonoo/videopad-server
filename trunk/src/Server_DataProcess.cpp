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

    CDataConnection* pNewConnection = new CDataConnection( nNewSocket );

    m_UnassignedDataConnectionVector.push_back( pNewConnection );
}

int CServer::ReadDataConnection( CDataConnection* pConnection )
{
    int res = recv( pConnection->GetSocket(),
	pConnection->GetOggDecoderBuffer( MAXDATAREAD ), MAXDATAREAD,
	MSG_DONTWAIT );

    if( ( res <= 0 ) && ( errno != EAGAIN ) )
    {
	// socket error, deleting client
	//
	for( vector< CClient* >::iterator it = m_ClientVector.begin();
	    it != m_ClientVector.end(); it++ )
	{
	    if( (*it)->GetDataConnection()->GetSocket() == pConnection->GetSocket() )
	    {
		DeleteClient( *it );
		break;
	    }
	}
	return -1;
    }

    pConnection->OggDecoderWrote( res );

    int nSerial = -1;
    ogg_page* pOggPage = NULL;
    CClient* pClient = NULL;

    do
    {
	pOggPage = pConnection->OggDecoderPageOut();
	if( pOggPage == NULL )
	{
	    // this happens when we could not associate a client
	    // with available ogg pages
	    // maybe there was unprocessed ogg pages in the decoder left from
	    // a previous data connection
	    //
//	    cout << res << ": ogg page fail" << endl;
	    return -2;
	}
//	cout << res << ": not failed" << endl;
	nSerial = ogg_page_serialno( pOggPage );

	// getting the client of this ogg page
	//
	pClient = m_SerialMapper.GetClient( nSerial );
    } while( pClient == NULL );

    if( pClient->GetDataConnection() == NULL )
    {
	pClient->SetDataConnection( pConnection );
    }

    // feeding ogg page into the client's associated stream
    //
    COggStream* pOggStream = m_SerialMapper.GetOggStream( nSerial );
    if( pOggStream == NULL )
    {
        return -1;
    }
    pOggStream->FeedPage( pOggPage );

    // processing page with the client
    ProcessClientData( pClient );

    return 0;
}

void CServer::ProcessClientData( CClient*& pClient )
{
    if( pClient == NULL )
    {
	return;
    } 

    COggStream* pOggStream = m_SerialMapper.GetOggStream(
        pClient->GetCurrentPageSerial() );

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
